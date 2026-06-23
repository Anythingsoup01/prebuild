#include "Platform.h"
#include "Platform/CMake/CMakePlatform.h"
#include "pbpch.h"

void stackdump(lua_State *L) {
  int i;
  int top = lua_gettop(L);
  std::cout << "--- Stack Dump (Top: " << top << ") ---" << std::endl;
  for (i = 1; i <= top; i++) {
    int t = lua_type(L, i);
    std::cout << i << ": ";
    switch (t) {
    case LUA_TSTRING:
      std::cout << "string: '" << lua_tostring(L, i) << "'";
      break;
    case LUA_TBOOLEAN:
      std::cout << "boolean: " << (lua_toboolean(L, i) ? "true" : "false");
      break;
    case LUA_TNUMBER:
      std::cout << "number: " << lua_tonumber(L, i);
      break;
    case LUA_TTABLE:
      std::cout << "table: (addr:" << lua_topointer(L, i) << ")";
      break;
    case LUA_TNIL:
      std::cout << "nil";
      break;
    default:
      std::cout << lua_typename(L, t);
      break;
    }
    std::cout << std::endl;
  }
  std::cout << "--- End Stack Dump ---" << std::endl;
}

namespace Prebuild {
static const char *PathKeywords[1] = {
    "${WORKSPACEDIR}",
};

struct Optional {
  int Value = 0;
  bool HasChanged = false;

  Optional operator++(int) {
    ++Value;
    HasChanged = true;
    return *this;
  }

  Optional operator--(int) {
    --Value;
    HasChanged = true;
    return *this;
  }

  operator bool() { return HasChanged; }
};

bool Platform::StrEqual(const std::string &in, const std::string &check) {
  if (strncmp(in.c_str(), check.c_str(), check.length()) == 0)
    return true;
  return false;
}

std::vector<std::string> GetTableVariables(lua_State *L,
                                           const std::string variableName) {
  std::vector<std::string> out;
  if (!lua_istable(L, -1) && lua_isstring(L, -1)) {
    out.push_back(lua_tostring(L, -1));
  } else {
    lua_pushnil(L);
    int index = lua_gettop(L) - 1;

    while (lua_next(L, index) != 0) {
      if (lua_isstring(L, -1)) {
        out.push_back(lua_tostring(L, -1));
      }
      lua_pop(L, 1);
    }
  }
  return out;
}

std::string GetStringVariable(lua_State *L, const std::string &variableName) {
  std::vector<std::string> out = GetTableVariables(L, variableName);
  return out[0];
}

Platform::Platform(const Utils::System &system,
                   const std::filesystem::path &searchDirectory)
    : m_System(system), m_SearchDirectory(searchDirectory) {
  // Create Lua State for liblua
  lua_State *state = luaL_newstate();
  if (!state) {
    std::cerr << "Failed to create lua state/\n";
    return;
  }

  // Open all required (standard) libraries
  luaL_openlibs(state);

  // Prebuild file in the current directory
  std::filesystem::path filePath = m_SearchDirectory / "prebuild.lua";

  // Parsing each block, primarily each table
  std::vector<std::string> luaBlocks = ParseLuaBlocks(filePath);
  if (luaBlocks.empty())
    return;

  // Setting up the file config "tree"
  std::vector<ProjectFileConfig> fileConfigs = {};

  // Creating the root node
  ProjectFileConfig rootFile = {};
  bool workspace_initialized = false;
  for (auto& block : luaBlocks) {
    if (luaL_dostring(state, block.c_str()) != LUA_OK) {
    std::cerr << "Failed to execute lua script: " << lua_tostring(state, -1)
              << std::endl;
      continue;
    }

    if (StrEqual(block, "Workspace") && !workspace_initialized) {
      m_WorkspaceConfig = GetWorkspaceVariables(state);
      m_WorkspaceConfig.WorkingDirectory = m_SearchDirectory;
      workspace_initialized = true;
    } else if (StrEqual(block, "Project") || StrEqual(block, "External")) {
      rootFile.Projects.push_back(GetProjectVariables(state));
    } else if (StrEqual(block, "External")) {
      std::string external = GetStringVariable(state, "External");
      rootFile.Externals.push_back(external);
      std::filesystem::path relativePath = m_SearchDirectory / external;
      m_TMPPaths.push_back(relativePath);
    }

    lua_pop(state, 1);
  }

  rootFile.Directory = m_SearchDirectory;
  fileConfigs.push_back(rootFile);

  // Clear for reuse
  luaBlocks.clear();

  while (!m_TMPPaths.empty()) {
    m_ExternalPaths = m_TMPPaths;
    m_TMPPaths.clear();

    // Sub paths (Externals) should be, for the most part, exactly the same as above
    for (auto &path : m_ExternalPaths) {
      luaBlocks = ParseLuaBlocks(path);
      ProjectFileConfig rootFile = {};
      for (auto& block : luaBlocks) {
        if (luaL_dostring(state, block.c_str()) != LUA_OK) {
          std::cerr << "Failed to execute lua script: " << lua_tostring(state, -1)
            << std::endl;
          continue;
        }

        if (StrEqual(block, "Project")) {
          rootFile.Projects.push_back(GetProjectVariables(state));
        } else if (StrEqual(block, "External")) {
          std::string external = GetStringVariable(state, "External");
          rootFile.Externals.push_back(external);
          std::filesystem::path relativePath = m_SearchDirectory / external;
          m_TMPPaths.push_back(relativePath);
        }

        lua_pop(state, 1);
      }

      rootFile.Directory = m_SearchDirectory;
      fileConfigs.push_back(rootFile);
    }
  }

  auto platform = Create(m_WorkspaceConfig, fileConfigs);
}

std::vector<std::string> Platform::ParseLuaBlocks(const std::filesystem::path &filePath) {
  std::ifstream in(filePath);
  if (!in.is_open()) {
    std::cout << "Failed to open: " << filePath.generic_string()
              << ": Assuming it's a Non-Prebuild File!" << std::endl;
    return {};
  }
  std::stringstream ss;
  ss << in.rdbuf();
  in.close();

  std::vector<std::string> out;

  std::string line;
  // If 0 (after already being 1) then we end the block
  uint16_t bracketCount = 0;
  std::string block = "";
  while (getline(ss, line)) {
    line.erase(remove_if(line.begin(), line.end(), isspace), line.end());
    bool external = strncmp(line.c_str(), "External", strlen("External")) == 0;

    if (line.find("{") != NPOS) {
      bracketCount++;
    } else if (line.find("}") != NPOS) {
      bracketCount--;
    }

    block.append(line);

    if (bracketCount == 0 || external) {
      out.push_back(block);
      block.clear();
    }

  }

  return out;
}

Platform::WorkspaceConfig Platform::GetWorkspaceVariables(lua_State *L) {
  WorkspaceConfig cfg;
  lua_getglobal(L, "Workspace");

  if (!lua_istable(L, -1)) {
    std::cerr << "Workspace must be a table!\n";
    return cfg;
  }

  lua_pushnil(L);
  int tableIndex = lua_gettop(L) - 1;

  while (lua_next(L, tableIndex) != 0) {
    std::string key = lua_tostring(L, -2);
    if (StrEqual(key, "name")) {
      cfg.Name = GetStringVariable(L, "name");
    } else if (StrEqual(key, "architecture")) {
      cfg.Architecture = StringToArchitectureType(GetStringVariable(L, "architecture"));
    } else if (StrEqual(key, "configurations")) {
      cfg.Configurations = GetTableVariables(L, "configurations");
    } else if (StrEqual(key, "defines")) {
      cfg.Defines = GetTableVariables(L, "defines");
    } else if (StrEqual(key, "flags")) {
      cfg.CompileFlags = GetTableVariables(L, "flags");
    }
    lua_pop(L, 1);
  }

  lua_pop(L, 1);

  if (cfg.Configurations.empty()) {
    cfg.Configurations = {"Debug", "Release"};
  }
  return cfg;
}

Platform::ProjectConfig Platform::GetProjectVariables(lua_State *L) {
  ProjectConfig cfg;
  lua_getglobal(L, "Project");

  if (!lua_istable(L, -1)) {
    std::cerr << "Project must be a table!\n";
    return cfg;
  }

  lua_pushnil(L);
  int tableIndex = lua_gettop(L) - 1;

  while (lua_next(L, tableIndex) != 0) {
    std::string key = lua_tostring(L, -2);
    if (StrEqual(key, "name")) {
      cfg.Name = GetStringVariable(L, "name");
    } else if (StrEqual(key, "language")) {
      cfg.Language = StringToLanguageType(GetStringVariable(L, "language"));
    } else if (StrEqual(key, "dialect")) {
      cfg.Dialect = GetStringVariable(L, "dialect");
    } else if (StrEqual(key, "kind")) {
      cfg.Kind = StringToKindType(GetStringVariable(L, "kind"));
    } else if (StrEqual(key, "pch")) {
      cfg.PrecompiledHeader = GetStringVariable(L, "pch");
    } else if (StrEqual(key, "files")) {
      std::vector<std::string> tmp = GetTableVariables(L, "files");
      for (auto file : tmp) {
        cfg.Files.push_back(file);
      }
    } else if (StrEqual(key, "includedirs")) {
      std::vector<std::string> tmp = GetTableVariables(L, "includedirs");
      for (auto file : tmp) {
        cfg.IncludedDirectories.push_back(file);
      }
    } else if (StrEqual(key, "links")) {
      cfg.Links = GetTableVariables(L, "links");
    } else if (StrEqual(key, "defines")) {
      cfg.Defines = GetTableVariables(L, "links");
    } else if (StrEqual(key, "flags")) {
      cfg.CompileFlags = GetTableVariables(L, "flags");
    }

    else if (StrEqual(key, "filters")) {
      if (lua_istable(L, -1)) {
        lua_pushnil(L);
        int configIndex = lua_gettop(L) - 1;

        while (lua_next(L, configIndex) != 0) {
          cfg.Filters.push_back(ProcessFilter(L));
          lua_pop(L, 1);
        }
      }
    }

    lua_pop(L, 1);
  }

  lua_pop(L, 1);
  return cfg;
}

Platform::FilterConfig Platform::ProcessFilter(lua_State *L) {
  FilterConfig cfg;
  if (lua_istable(L, -1)) {
    lua_pushnil(L);
    int configIndex = lua_gettop(L) - 1;

    while (lua_next(L, configIndex) != 0) {
      std::string key = lua_tostring(L, -2);

      if (StrEqual(key, "name")) {
        cfg.Name = GetStringVariable(L, "name");
      } else if (StrEqual(key, "files")) {
        std::vector<std::string> tmp = GetTableVariables(L, "files");
        for (auto &file : tmp)
          cfg.Files.push_back(file);
      } else if (StrEqual(key, "defines")) {
        cfg.Defines = GetTableVariables(L, "defines");
      } else if (StrEqual(key, "flags")) {
        cfg.CompileFlags = GetTableVariables(L, "flags");
      } else if (StrEqual(key, "links")) {
        cfg.Links = GetTableVariables(L, "links");
      }

      lua_pop(L, 1);
    }
  }
  return cfg;
}

Platform::ArchitectureType
Platform::StringToArchitectureType(const std::string &archStr) {
  if (StrEqual(archStr, "x86"))
    return ArchitectureType::X86;
  if (StrEqual(archStr, "x64"))
    return ArchitectureType::X64;

  return ArchitectureType::NONE;
}

Platform::LanguageType
Platform::StringToLanguageType(const std::string &langStr) {
  if (langStr == "C")
    return LanguageType::C;
  if (langStr == "C++")
    return LanguageType::CXX;
  return LanguageType::NONE;
}

Platform::KindType Platform::StringToKindType(const std::string &kindStr) {
  if (kindStr == "StaticLib")
    return KindType::STATICLIB;
  if (kindStr == "SharedLib")
    return KindType::SHAREDLIB;
  if (kindStr == "ConsoleApp")
    return KindType::CONSOLEAPP;
  return KindType::NONE;
}

Scope<Platform>
Platform::Create(const WorkspaceConfig &workspaceConfig,
                 const std::vector<ProjectFileConfig> &fileConfigs) {
  switch (m_System) {
  case Utils::System::CMAKE:
    return nullptr;// CreateScope<CMakePlatform>(workspaceConfig, fileConfigs);
  default:
    return nullptr;
  }
}
} // namespace Prebuild
