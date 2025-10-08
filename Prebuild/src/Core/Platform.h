#pragma once
#include "Core.h"
#include "Core/Utils.h"


#include <lua.hpp>


const size_t NPOS = std::string::npos;

namespace Prebuild
{
    class Platform
    {
    public:
        enum class ProjectType
        {
            NONE = 0,
            INLINE,
            EXTERNAL,
        };

        enum class KindType
        {
            NONE = 0,
            STATICLIB,
            SHAREDLIB,
            CONSOLEAPP,
        };

        enum class LanguageType
        {
            NONE = 0,
            C,
            CXX,
        };

        enum class ArchitectureType
        {
            NONE = 0,
            X86,
            X64,
        };

        enum class KeywordType
        {
            NONE = 0,
            WORKSPACE,
            PROJECT,
            FILTER,
            FILEPATH,
        };
    public:
        struct WorkspaceConfig
        {
            std::filesystem::path WorkingDirectory;
            std::string Name;
            ArchitectureType Architecture;

            std::vector<std::string> Configurations;
            std::vector<std::string> Defines;

            std::vector<std::string> Externals;
        };

        struct FilterConfig
        {
            std::string Name;
            std::vector<std::filesystem::path> Files;
            std::vector<std::string> Defines;
            std::vector<std::string> Links;
        };

        struct ProjectConfig
        {
            std::filesystem::path Directory;
            std::string Name;
            std::string Dialect;

            LanguageType Language;
            KindType Kind;

            std::string PrecompiledHeader;

            bool External;

            std::vector<std::filesystem::path> Files;
            std::vector<std::filesystem::path> IncludedDirectories;
            std::vector<std::string> Links;
            std::vector<std::string> Defines;

            std::vector<FilterConfig> Filters;

            std::vector<std::string> Externals;
        };

        WorkspaceConfig m_WorkspaceConfig;
        std::vector<ProjectConfig> m_Projects;

    public:
        Platform() = default;
        Platform(const Utils::System& system, const std::filesystem::path& searchDirectory = std::filesystem::current_path());
        ~Platform() {}
        static bool StrEqual(const std::string& in, const std::string& check);
    private:

        std::string FormatFileToLua(const std::filesystem::path& filePath);

        WorkspaceConfig GetWorkspaceVariables(lua_State* L);
        ProjectConfig GetInlineProject(lua_State* L, const std::string& varName);
        ProjectConfig GetExternalProject(const std::filesystem::path& path);
        ProjectConfig GetProjectVariables(lua_State* L, const std::string& varName, const std::filesystem::path& path);

        FilterConfig ProcessFilter(lua_State* L);

        ArchitectureType StringToArchitectureType(const std::string& archStr);
        LanguageType StringToLanguageType(const std::string& langStr);
        KindType StringToKindType(const std::string& kindStr);


        Scope<Platform> Create(const WorkspaceConfig& workspaceConfig, const std::vector<ProjectConfig>& projectConfigs);
    private:
        Utils::System m_System;
        std::filesystem::path m_SearchDirectory;

        std::vector<std::filesystem::path> m_ExternalPaths;
        std::vector<std::filesystem::path> m_TMPPaths;

        int m_CurrentProjectCount = 0;
        int m_CurrentExternalCount = 0;
    };
}
