#include "pbpch.h"

#include "CMakePlatform.h"
#include "Core/Utils.h"

#include <array>

#ifdef PB_DEBUG
#define PB_PRINT(x) printf("FUNCTION - %s\n", x)
#else
#define PB_PRINT(x)
#endif

namespace Prebuild {
std::string execute_command(const char *command) {
  std::array<char, 128> buffer;
  std::string result;
  // Use popen for POSIX and _popen for Windows
#ifdef _WIN32
  std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command, "r"), _pclose);
#else
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
#endif
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

CMakePlatform::CMakePlatform(const WorkspaceConfig &workspaceConfig,
                             const std::vector<ProjectFileConfig> &fileConfigs)
    : m_WorkspaceConfig(workspaceConfig), m_FileConfigs(fileConfigs) {
  std::string tmp = execute_command("cmake --version");
  size_t eol = tmp.find_first_of("\n");
  tmp.erase(eol);
  m_Version = tmp.substr(strlen("cmake version "), eol);

  Build();
}
// BUILDING CMAKELISTS
// -----------------------------------------------------------------------

void CMakePlatform::Build() {
  bool containsC = false;
  bool containsCXX = false;

  // Instead of using stringstream to hold the entire file buffer, we will
  // break it into each "chunk" so it can be appended correctly
  std::vector<std::string> rootFileBlocks = {};


  // Looping over each File Config
  for (auto &cfg : m_FileConfigs) {
    std::stringstream ss;
    // Looping over each project in the File Config
    for (auto &proj : cfg.Projects) {
      // These are a global so the entire project can compile dependencies of different languages and link them
      if (proj.Language == LanguageType::C) containsC = true;
      if (proj.Language == LanguageType::CXX) containsCXX = true;
      std::string block = BuildProject(proj, cfg.Directory);

      if (cfg.Directory.string() == m_WorkspaceConfig.WorkingDirectory.string()) {
        rootFileBlocks.push_back(block);
      } else {
        ss << block << "\n\n";
      }
    }

    for (auto &ext : cfg.Externals) {
      std::string block = "add_subdirectory(" + ext + ")\n";

      if (cfg.Directory.string() == m_WorkspaceConfig.WorkingDirectory.string()) {
        rootFileBlocks.push_back(block);
      } else {
        ss << block << "\n\n";
      }
    }

    if (ss.str().empty()) {
      continue;
    }

    std::ofstream out(cfg.Directory / "CMakeLists.txt");
    if (!out.is_open()) {
      std::cerr << "failed to generate - "
        << (cfg.Directory / "CMakeLists.txt").generic_string()
        << "\n";
      continue;
    }
    out << ss.rdbuf();
    out.close();
  }

  std::stringstream ss;
  ss << "cmake_minimum_required(VERSION " << m_Version << ")\n"
     << "set(CMAKE_EXPORT_COMPILE_COMMANDS ON)\n"
     << "project(" << m_WorkspaceConfig.Name << " LANGUAGES";
  if (containsC)
    ss << " C";
  if (containsCXX)
    ss << " CXX";
  ss << ")\n";
  if (!m_WorkspaceConfig.Defines.empty()) {
    ss << "set(GDEFINES\n";
    for (auto &def : m_WorkspaceConfig.Defines)
      ss << "    " << def << std::endl;
    ss << ")\n\n";
  }
  if (!m_WorkspaceConfig.CompileFlags.empty()) {
    ss << "add_compile_options(\n";
    for (auto &flag : m_WorkspaceConfig.CompileFlags)
      ss << "    " << flag << "\n";
    ss << ")\n\n";
  }

  std::ofstream out(m_WorkspaceConfig.WorkingDirectory / "CMakeLists.txt");
  if (!out.is_open()) {
    std::stringstream err;
    Utils::PrintWarning("Could not generate root CMakeLists.txt");
    return;
  }

  for (auto &block : rootFileBlocks) {
    ss << block << "\n";
  }

  out << ss.str();
  out.close();
}

std::string CheckKeyword(const std::filesystem::path &rootPath) {
  if (Platform::StrEqual(rootPath.string(), "${WORKSPACEDIR}"))
    return "${CMAKE_SOURCE_DIR}";
  return std::string();
}

std::string RecursiveSearchForExtension(const std::filesystem::path &workingDir, const std::filesystem::path &searchPath, const std::string &keyword, const std::string &extension) {
  std::stringstream out;
  try {
    for (const auto &entry : std::filesystem::recursive_directory_iterator(searchPath)) {
      if (entry.path().extension() == extension) {
        std::filesystem::path relPath = std::filesystem::relative(entry.path(), workingDir);
        // TODO: Handle multiple keywords when the time comes
        if (!keyword.empty()) {
          out << keyword << "/";
        }
        out << relPath.generic_string() << "\n";
      }
    }
  } catch (std::filesystem::filesystem_error e) {
    throw std::filesystem::filesystem_error(e.what(), e.path1(), e.path2(), e.code());
  }
  return out.str();
}

std::string SearchForExtension(const std::filesystem::path &workingDir, const std::filesystem::path &searchPath, const std::string &keyword, const std::string &extension) {
  std::stringstream out;
  try {
    for (const auto &entry : std::filesystem::directory_iterator(searchPath)) {
      if (entry.path().extension() == extension) {
        std::filesystem::path relPath = std::filesystem::relative(entry.path(), workingDir);
        // TODO: Handle multiple keywords when the time comes
        if (!keyword.empty()) {
          out << keyword << "/";
        }
        out << relPath.generic_string() << "\n";
      }
    }
  } catch (std::filesystem::filesystem_error e) {
    throw std::filesystem::filesystem_error(e.what(), e.path1(), e.path2(), e.code());
  }
  return out.str();

}

std::string CMakePlatform::BuildProjectFiles(const std::string &projName, const std::vector<std::filesystem::path>& files, const std::filesystem::path &relPath) {
  std::stringstream out;

  out << "target_sources(" << projName << " PRIVATE\n";
  for (auto &file : files) {
    std::string keyword = CheckKeyword(*file.begin());
    std::filesystem::path parsedPath = file;
    if (!keyword.empty()) {
      bool first = true;
      for (auto &part : file) {
        if (first) {
          first = false;
          parsedPath = keyword;
          continue;
        }
        parsedPath /= part;
      }
    }

    bool recursiveSearch = false;
    if (Platform::StrEqual(parsedPath.filename().string(), "**")) {
      recursiveSearch = true;
    } else if (!StrEqual(parsedPath.filename().string(), "*")) {
      out << parsedPath << "\n";
      continue;
    }

    std::filesystem::path searchPath;
    // TODO: Handle mutiple preprocessor keywords
    if (Platform::StrEqual(keyword, "${WORKSPACEDIR}")) {
      searchPath = m_WorkspaceConfig.WorkingDirectory / file.parent_path();
    } else {
      searchPath = relPath / file.parent_path();
    }

    try {
      if (recursiveSearch) out << RecursiveSearchForExtension(relPath, searchPath, keyword, file.extension());
      else SearchForExtension(relPath, searchPath, keyword, file.extension());
    } catch (std::filesystem::filesystem_error e) {
      std::cerr << "Failed to find directory:\n"
                << "What: " << e.what() << "\n"
                << "Path 1: " << e.path1() << "\n"
                << "Path 2: " << e.path2() << "\n"
                << "Error Code: " << e.code();

    }
  }

  out << ")\n";

  return out.str();
}

std::string CMakePlatform::BuildProjectKind(const std::string &projName, const KindType &projKind) {
  std::stringstream out;
  switch (projKind) {
    case KindType::STATICLIB: {
      out << "add_library(" << projName << ")\n";
      break;
    }
    case KindType::SHAREDLIB: {
      out << "add_library(" << projName << ")\n";
      break;
    }
    case KindType::CONSOLEAPP: {
      out << "add_executable(" << projName << ")\n";
      break;
    }
  }
  return out.str();
}

std::string CMakePlatform::BuildProjectLanguage(const std::string &projName, const LanguageType &projLang, const std::string &projDialect) {
  std::stringstream out;

  out << "set_property(TARGET " << projName << " ";
  switch (projLang) {
    case LanguageType::C: {
      out << "PROPERTY C_STANDARD " << projDialect;
    break;
    }
    case LanguageType::CXX: {
      out << "PROPERTY CXX_STANDARD " << projDialect;
      break;
    }
  }
  out << ")\n";
  return out.str();
}

std::string CMakePlatform::BuildProjectIncludeDirs(const std::string &projName, const std::vector<std::filesystem::path> &projIncludeDirs) {
  std::stringstream out;
  out << "target_include_directories(" << projName << " PRIVATE\n";
  for (auto &dir : projIncludeDirs) {
    std::string keyword = CheckKeyword(*dir.begin());
    std::filesystem::path parsedPath = dir;
    if (!keyword.empty()) {
      bool first = true;
      for (auto &path : dir) {
        if (first) {
          first = false;
          parsedPath = keyword;
          continue;
        }
        parsedPath /= path;
      }
    }
    out << parsedPath.generic_string() << "\n";
  }

  out << ")\n";

  return out.str();
}

std::string CMakePlatform::BuildProjectLinks(const std::string &projName, const std::vector<std::string> projLinks) {
  std::stringstream out;
  out << "target_link_libraries(" << projName << "\n";

  for (auto &src : projLinks) {
    out << src << "\n";
  }

  out << ")\n";
  return out.str();
}

std::string CMakePlatform::BuildProjectFlags(const std::string &projName, const std::vector<std::string> projFlags) {
  std::stringstream out;
  out << "target_compile_options(" << projName << " PRIVATE\n";
  for (auto &flag : projFlags)
    out << flag << "\n";
  out << ")\n";
  return out.str();
}

std::string CMakePlatform::BuildProjectDefines(const std::string &projName, const std::vector<std::string> projDefines) {
  std::stringstream out;
  out << "target_compile_definitions(" << projName << " PUBLIC\n";
  for (auto &define : projDefines)
    out << define << "\n";
  out << ")\n";
  return out.str();
}

std::string CMakePlatform::BuildProject(const ProjectConfig &proj, const std::filesystem::path &relPath) {
  std::stringstream out;

  out << BuildProjectKind(proj.Name, proj.Kind) << "\n";
  out << BuildProjectLanguage(proj.Name, proj.Language, proj.Dialect);

  if (!proj.Files.empty()) {
    out << BuildProjectFiles(proj.Name, proj.Files, relPath) << "\n";
  }
 
  if (!proj.PrecompiledHeader.empty()) {
    out << "target_precompile_headers(" << proj.Name << " PUBLIC "
       << proj.PrecompiledHeader << ")\n\n";
  }

  if (!proj.IncludedDirectories.empty()) {
    out << BuildProjectIncludeDirs(proj.Name, proj.IncludedDirectories) << "\n";
  }

  if (!proj.Links.empty()) {
    out << BuildProjectLinks(proj.Name, proj.Links) << "\n";
  }
  if (!proj.CompileFlags.empty()) {
    out << BuildProjectFlags(proj.Name, proj.CompileFlags) << "\n";
  }

  if (!proj.Defines.empty()) {
    out << BuildProjectDefines(proj.Name, proj.CompileFlags) << "\n";
  }

  if (!m_WorkspaceConfig.Defines.empty() && !proj.Name.empty()) {
    out << "target_compile_definitions(" << proj.Name << " PUBLIC GDEFINES)\n";
  }

  for (auto &filter : proj.Filters)
    out << BuildFilter(filter);

  return out.str();
}

std::string CMakePlatform::StartFilterSystem(const FilterConfig &cfg) {
  std::stringstream out;

  std::string platform;
  if (StrEqual(cfg.Param, "linux"))
    platform = "UNIX AND NOT APPLE";
  if (StrEqual(cfg.Param, "windows"))
    platform = "WIN32";

  out << "if (" << platform << ")\n";
  return out.str();
}

std::string CMakePlatform::StartFilterConfiguration(const FilterConfig &cfg) {
  return "if(CMAKE_BUILD_TYPE STREQUAL " + cfg.Param + ")\n";
}

std::string CMakePlatform::BuildFilter(const FilterConfig &cfg) {
  std::stringstream out;

  bool isSystem = StrEqual(cfg.Type, "system");
  bool isConfig = isSystem ? false : StrEqual(cfg.Type, "configuration");

  if (!isSystem && !isConfig) return std::string();

  if (isSystem) {
    out << StartFilterSystem(cfg) << "\n";
  } else {
    out << StartFilterConfiguration(cfg) << "\n";
  }

  if (!cfg.Files.empty()) {
    out << BuildProjectFiles(cfg.ParentProject->Name, cfg.Files, cfg.ParentProject->WorkingDirectory) << "\n";
  }

  if (!cfg.Defines.empty()) {
    out << BuildProjectDefines(cfg.ParentProject->Name, cfg.Defines) << "\n";
  }

  if (!cfg.Links.empty()) {
    out << BuildProjectLinks(cfg.ParentProject->Name, cfg.Links) << "\n";
  }

  if (!cfg.CompileFlags.empty()) {
    out << BuildProjectFlags(cfg.ParentProject->Name, cfg.CompileFlags) << "\n";
  }

  out << "endif()\n";

  return out.str();
}

} // namespace Prebuild
