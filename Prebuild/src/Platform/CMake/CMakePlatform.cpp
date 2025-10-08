#include "pbpch.h"

#include "CMakePlatform.h"
#include "Core/Utils.h"

#include <array>

#ifdef PB_DEBUG
#define PB_PRINT(x) printf("FUNCTION - %s\n", x)
#else
#define PB_PRINT(x)
#endif

namespace Prebuild
{
    std::string execute_command(const char* command) 
    {
        std::array<char, 128> buffer;
        std::string result;
        // Use popen for POSIX and _popen for Windows
#       ifdef _WIN32
        std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command, "r"), _pclose);
#       else
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
#       endif
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }

    CMakePlatform::CMakePlatform(const WorkspaceConfig& workspaceConfig, const std::vector<ProjectConfig>& projects)
            : m_WorkspaceConfig(workspaceConfig), m_Projects(projects)
    {
        std::string tmp = execute_command("cmake --version");
        size_t eol = tmp.find_first_of("\n");
        tmp.erase(eol);
        m_Version = tmp.substr(strlen("cmake version "), eol);

        Build();
    }
    // BUILDING CMAKELISTS -----------------------------------------------------------------------

    void CMakePlatform::Build()
    {
        std::stringstream ss;

        bool containsC = false;
        bool containsCXX = false;

        for (auto& cfg : m_Projects)
        {
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
        if (!m_WorkspaceConfig.Defines.empty())
        {
            ss << "set(GDEFINES\n";
            for (auto& def : m_WorkspaceConfig.Defines)
               ss << "    " << def << std::endl;
            ss << ")\n\n";
        }


        for (const auto& cfg : m_Projects)
        {
            if (!cfg.External)
            {
                ss << BuildProject(cfg);
            }
            else
            {
                std::stringstream ess;
                ess << BuildProject(cfg);

                std::ofstream out(cfg.Directory / "CMakeLists.txt");
                if (!out.is_open())
                {
                    std::cerr << "failed to generate - " << (cfg.Directory / "CMakeLists.txt").generic_string() << std::endl;
                    continue;
                }
                out << ess.rdbuf();
                out.close();
            }
        }


        for (auto& external : m_WorkspaceConfig.Externals)
            ss << "add_subdirectory(" << external << ")\n";

        std::ofstream out(m_WorkspaceConfig.WorkingDirectory / "CMakeLists.txt");
        if (!out.is_open())
        {
            std::stringstream err;
            Utils::PrintWarning("Could not generate root CMakeLists.txt");
            return;
        }
        out << ss.str();
        out.close();

    }

    std::filesystem::path ContainsWorkspaceKeyword(std::filesystem::path& file)
    {
        if (CMakePlatform::StrEqual(file.begin()->string(), "${WORKSPACEDIR}"))
        {
            std::string tmp = file.generic_string();
            tmp.erase(0, strlen("${WORKSPACEDIR}"));
            return tmp;
        }
        return file;
    }

    std::filesystem::path ReplaceWorkspaceKeyword(const std::filesystem::path& file)
    {
        if (CMakePlatform::StrEqual(file.begin()->string(), "${WORKSPACEDIR}"))
        {
            std::string tmp = file.generic_string();
            tmp.erase(0, strlen("${WORKSPACEDIR}"));
            return std::filesystem::path("${CMAKE_SOURCE_DIR}") / tmp;
        }
        return file;

    }

    std::string CMakePlatform::BuildProject(const ProjectConfig& cfg)
    {
        std::stringstream ss;

        if (!cfg.Files.empty())
        {
            ss << "set(SRCS\n";

            std::vector<std::filesystem::path> tmp;

            for (auto file : cfg.Files)
            {
                if (!StrEqual(file.filename().string(), "*"))
                {
                    tmp.push_back(file);
                    continue;
                }

                bool hasKeyword = false;

                std::filesystem::path tmpFile = ContainsWorkspaceKeyword(file);

                if (!StrEqual(tmpFile.generic_string(), file.generic_string()))
                {
                    hasKeyword = true;
                    file = tmpFile;
                }


                std::filesystem::path actualPath;
                if (!cfg.Directory.has_parent_path() || hasKeyword)
                    actualPath = m_WorkspaceConfig.WorkingDirectory;
                else
                    actualPath = cfg.Directory;

                std::filesystem::path relPath = actualPath;

                if (file.has_parent_path())
                    actualPath /= file.parent_path();

                try
                {
                    for (const auto& entry : std::filesystem::recursive_directory_iterator(actualPath))
                    {
                        if (entry.path().extension() == file.extension())
                        {
                            std::filesystem::path relativePath = std::filesystem::relative(entry.path(), relPath);
                            std::filesystem::path pushBack;
                            if (hasKeyword)
                            {
                                pushBack /= "${CMAKE_SOURCE_DIR}" / relativePath;
                            }
                            else
                            {
                                pushBack = relativePath;
                            }
                            tmp.push_back(pushBack);
                        }
                    }
                } catch (std::filesystem::filesystem_error) { std::cerr << "Failed to find directory: '" << actualPath.generic_string() << "' : for project: '" << cfg.Name << "'!\n"; }
            }

            for (auto& src : tmp)
            {
                ss << "    " << src << std::endl;
            }
            ss << ")\n";
        }

        switch (cfg.Kind)
        {
            case KindType::STATICLIB:
            {
                ss << "add_library(" << cfg.Name << " STATIC ${SRCS})\n";
                break;
            }
            case KindType::SHAREDLIB:
            {
                ss << "add_library(" << cfg.Name << " SHARED ${SRCS})\n";
                break;
            }
            case KindType::CONSOLEAPP:
            {
                ss << "add_executable(" << cfg.Name << " ${SRCS})\n";
                break;
            }
            default:
                break;
        }

        switch (cfg.Language)
        {
            case LanguageType::C:
            {
                ss << "set_property(TARGET " << cfg.Name << " PROPERTY C_STANDARD " << cfg.Dialect << ")\n\n";
                break;
            }
            case LanguageType::CXX:
            {
                ss << "set_property(TARGET " << cfg.Name << " PROPERTY CXX_STANDARD " << cfg.Dialect << ")\n\n";
                break;
            }
            default:
                break;
        }

        if (!cfg.PrecompiledHeader.empty())
        {
            ss << "target_precompile_headers(" <<cfg.Name << " PUBLIC " << cfg.PrecompiledHeader << ")\n";
        }


        if (!cfg.IncludedDirectories.empty())
        {
            ss << "target_include_directories(" << cfg.Name << " PRIVATE\n";

            for (auto& src :cfg.IncludedDirectories)
            {
                ss << "    " << ReplaceWorkspaceKeyword(src) <<  std::endl;
            }

            ss << ")\n\n";
        }

        if (!cfg.Links.empty())
        {
            ss << "target_link_libraries(" << cfg.Name << "\n";

            for (auto& src : cfg.Links)
            {
                ss << "    " << src <<  std::endl;
            }

            ss << ")\n\n";
        }

        if (!cfg.Defines.empty())
        {
            ss << "target_compile_definitions(" << cfg.Name << " PUBLIC\n";
            for (auto& define : cfg.Defines)
                ss << "    " << define << std::endl;
            ss << ")\n\n";
        }

        if (!m_WorkspaceConfig.Defines.empty() && !cfg.Name.empty())
        {
            ss << "target_compile_definitions(" << cfg.Name << " PUBLIC GDEFINES)\n";
        }

        for (auto& filter : cfg.Filters)
            ss << BuildFilter(filter, cfg.Name);

        for (auto& external : cfg.Externals)
            ss << "add_subdirectory(" << external << ")\n";

        return ss.str();
    }

    std::string CMakePlatform::BuildFilter(const FilterConfig& cfg, const std::string& target)
    {
        std::stringstream ss;

        if (cfg.Name.find("system") != NPOS)
            ss << BuildFilterPlatform(cfg, target);
        else if (cfg.Name.find("configurations") != NPOS)
            ss << BuildFilterConfigurations(cfg, target);

        return ss.str();
    }

    std::string CMakePlatform::BuildFilterPlatform(const FilterConfig& cfg, const std::string& target)
    {
        std::stringstream ss;
        std::string platform;
        std::string filter = cfg.Name;
        filter.erase(0, 7);
        if (filter == "linux")
            platform = "UNIX AND NOT APPLE";
        else if (filter == "windows")
            platform = "WIN32";

        ss << "if (" << platform << ")\n";
        if (!cfg.Files.empty())
        {
            ss << "target_sources(" << target << "PRIVATE\n";
            for (auto& file : cfg.Files)
                ss << "    " << file << std::endl;
            ss << ")\n";
        }
        if (!cfg.Defines.empty())
        {
            ss << "target_compile_definitions(" << target << "\nPUBLIC\n";
            for (auto& define : cfg.Defines)
                ss << "    " <<define << std::endl;
            ss << ")\n";
        }

        if (!cfg.Links.empty())
        {
            ss << "target_link_libraries(" << target << "\n";
            for (auto& link : cfg.Links)
                ss << "    " << link << std::endl;
            ss << ")\n";
        }

        ss << "endif(" << platform << ")\n";

        return ss.str();
    }

    std::string CMakePlatform::BuildFilterConfigurations(const FilterConfig& cfg, const std::string& target)
    {
        std::stringstream ss;
        std::string config;
        std::string filter = cfg.Name;
        filter.erase(0, 15);
        for (auto& configuration : m_WorkspaceConfig.Configurations)
        {
            if (filter == configuration)
                config = configuration;
        }

        if (config.empty())
        {
            std::stringstream msg;
            msg << "Configuration " << filter << " could not be built!\n"
                << "Please make sure it exists in the workspace!";
            Utils::PrintError(msg);
            return std::string();
        }
        ss << "if(CMAKE_BUILD_TYPE STREQUAL " << config << ")\n";
        if (!cfg.Defines.empty())
        {
            ss << "target_compile_definitions(" << target << "\nPUBLIC\n";
            for (auto& define : cfg.Defines)
                ss << "    " << define << std::endl;
            ss << ")\n";
        }

        if (!cfg.Links.empty())
        {
            ss << "target_link_libraries(" << target << "\n";
            for (auto& link : cfg.Links)
                ss << "    " << link << std::endl;
            ss << ")\n";
        }
        ss << "endif(CMAKE_BUILD_TYPE STREQUAL " << config << ")\n";


        return ss.str();
    }



}

