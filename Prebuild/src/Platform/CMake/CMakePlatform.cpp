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
  std::stringstream ss;

  bool containsC = false;
  bool containsCXX = false;

  // Instead of using stringstream to hold the entire file buffer, we will
  // break it into each "chunk" so it can be appended correctly
  std::vector<std::string> rootFileBlocks = {};


  // Looping over each File Config
  for (auto &cfg : m_FileConfigs) {
    std::vector<std::string> subFileBlocks = {};
    // Looping over each project in the File Config
    for (auto &proj : cfg.Projects) {
      // These are a global so the entire project can compile dependencies of different languages and link them
      if (proj.Language == LanguageType::C) containsC = true;
      if (proj.Language == LanguageType::CXX) containsCXX = true;
      std::string block = BuildProject(proj, cfg.Directory.parent_path());
    }
  }

  for (auto &cfg : m_Projects) {
    if (cfg.Language == LanguageType::C)
      containsC = true;
    if (cfg.Language == LanguageType::CXX)
      containsCXX = true;
  }

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

  for (const auto &cfg : m_Projects) {
    if (!cfg.External) {
      ss << BuildProject(cfg);
    } else {
      std::stringstream ess;
      ess << BuildProject(cfg);

      std::ofstream out(cfg.Directory / "CMakeLists.txt");
      if (!out.is_open()) {
        std::cerr << "failed to generate - "
                  << (cfg.Directory / "CMakeLists.txt").generic_string()
                  << std::endl;
        continue;
      }
      out << ess.rdbuf();
      out.close();
    }
  }

  for (auto &external : m_WorkspaceConfig.Externals)
    ss << "add_subdirectory(" << external << ")\n";

  std::ofstream out(m_WorkspaceConfig.WorkingDirectory / "CMakeLists.txt");
  if (!out.is_open()) {
    std::stringstream err;
    Utils::PrintWarning("Could not generate root CMakeLists.txt");
    return;
  }
  out << ss.str();
  out.close();
}

std::string CheckKeyword(const std::filesystem::path &rootPath) {
  if (Platform::StrEqual(rootPath.string(), "${WORKSPACEDIR}"))
    return "${CMAKE_SOURCE_DIR}";
  return std::string();
}

std::string RecursiveSearchForExtension(const std::filesystem::path &searchPath, const std::string &keyword, const std::string &extension) {
  std::stringstream out;
  try {
    for (const auto &entry : std::filesystem::recursive_directory_iterator(searchPath)) {
      if (entry.path().extension() == extension) {
        std::filesystem::path relativePath = std::filesystem::relative(entry.path(), relPath);
        // TODO: Handle multiple keywords when the time comes
        if (!keyword.empty()) {
          out << keyword << "/";
        }
        out << relativePath.generic_string() << "\n";
      }
    }
  } catch (std::filesystem::filesystem_error e) {
    throw std::filesystem::filesystem_error(e.what(), e.path1(), e.path2(), e.code());
  }
  return out.str();
}

std::string SearchForExtension(const std::filesystem::path &searchPath, const std::string &keyword, const std::string &extension) {
  std::stringstream out;
  try {
    for (const auto &entry : std::filesystem::directory_iterator(searchPath)) {
      if (entry.path().extension() == extension) {
        std::filesystem::path relativePath = std::filesystem::relative(entry.path(), relPath);
        // TODO: Handle multiple keywords when the time comes
        if (!keyword.empty()) {
          out << keyword << "/";
        }
        out << relativePath.generic_string() << "\n";
      }
    }
  } catch (std::filesystem::filesystem_error e) {
    throw std::filesystem::filesystem_error(e.what(), e.path1(), e.path2(), e.code());
  }
  return out.str();

}

//
//  Used to parse the files of a project config
//
std::string CMakePlatform::BuildProjectFiles(const std::vector<std::filesystem::path>& files, const std::filesystem::path &relPath) {
  std::stringstream out;

  out << "set(SRCS\n";
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
      if (recursiveSearch) out << RecursiveSearchForExtension(searchPath, keyword, file.extension());
      else SearchForExtension(searchPath, keyword, file.extension());
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
      out << "add_library(" << projName << " STATIC ${SRCS})\n";
      break;
    }
    case KindType::SHAREDLIB: {
      out << "add_library(" << projName << " SHARED ${SRCS})\n";
      break;
    }
    case KindType::CONSOLEAPP: {
      out << "add_executable(" << projName << " ${SRCS})\n";
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
    out << parsedPath << "\n";
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

  if (!proj.Files.empty()) {
    out << BuildProjectFiles(proj.Files, relPath) << "\n";
  }
  out << BuildProjectKind(proj.Name, proj.Kind) << "\n";
  out << BuildProjectLanguage(proj.Name, proj.Language, proj.Dialect);

 
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
    out << BuildFilter(filter, proj.Name);

  return out.str();
}

std::string CMakePlatform::BuildFilter(const FilterConfig &cfg,
                                       const std::string &target) {
  std::stringstream ss;

  bool isSystem = StrEqual(cfg.Type, "system");
  bool isConfig = isSystem ? false : StrEqual(cfg.Type, "configuration");

  if (!isSystem && !isConfig) return std::string();
  

  return ss.str();
}

std::string CMakePlatform::BuildFilterPlatform(const FilterConfig &cfg,
                                               const std::string &target) {
  std::stringstream ss;
  std::string platform;
  std::string filter = cfg.Name;
  filter.erase(0, 7);
  if (filter == "linux")
    platform = "UNIX AND NOT APPLE";
  else if (filter == "windows")
    platform = "WIN32";

  ss << "if (" << platform << ")\n";
  if (!cfg.Files.empty()) {
    ss << "target_sources(" << target << "PRIVATE\n";
    for (auto &file : cfg.Files)
      ss << "    " << file << std::endl;
    ss << ")\n";
  }
  if (!cfg.Defines.empty()) {
    ss << "target_compile_definitions(" << target << "\nPUBLIC\n";
    for (auto &define : cfg.Defines)
      ss << "    " << define << std::endl;
    ss << ")\n";
  }

  if (!cfg.Links.empty()) {
    ss << "target_link_libraries(" << target << "\n";
    for (auto &link : cfg.Links)
      ss << "    " << link << std::endl;
    ss << ")\n";
  }
  if (!cfg.CompileFlags.empty()) {
    ss << "target_compile_options(" << target << " PRIVATE\n";
    for (auto &flag : cfg.CompileFlags)
      ss << "    " << flag << "\n";
    ss << ")\n\n";
  }

  ss << "endif(" << platform << ")\n";

  return ss.str();
}

std::string
CMakePlatform::BuildFilterConfigurations(const FilterConfig &cfg,
                                         const std::string &target) {
  std::stringstream ss;
  std::string config;
  std::string filter = cfg.Name;
  filter.erase(0, 15);
  for (auto &configuration : m_WorkspaceConfig.Configurations) {
    if (filter == configuration)
      config = configuration;
  }

  if (config.empty()) {
    std::stringstream msg;
    msg << "Configuration " << filter << " could not be built!\n"
        << "Please make sure it exists in the workspace!";
    Utils::PrintError(msg);
    return std::string();
  }
  ss << "if(CMAKE_BUILD_TYPE STREQUAL " << config << ")\n";
  if (!cfg.Defines.empty()) {
    ss << "target_compile_definitions(" << target << "\nPUBLIC\n";
    for (auto &define : cfg.Defines)
      ss << "    " << define << std::endl;
    ss << ")\n";
  }

  if (!cfg.Links.empty()) {
    ss << "target_link_libraries(" << target << "\n";
    for (auto &link : cfg.Links)
      ss << "    " << link << std::endl;
    ss << ")\n";
  }
  if (!cfg.CompileFlags.empty()) {
    ss << "target_compile_options(" << target << " PRIVATE\n";
    for (auto &flag : cfg.CompileFlags)
      ss << "    " << flag << "\n";
    ss << ")\n\n";
  }
  ss << "endif(CMAKE_BUILD_TYPE STREQUAL " << config << ")\n";

  return ss.str();
}

} // namespace Prebuild
