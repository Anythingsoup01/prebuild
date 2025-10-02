#pragma once

#include "Core/Platform.h"
#include "Platform/CMake/CMakePlatform.h"

namespace Prebuild
{
    class NewCMakePlatform : public Platform
    {
    private:
        struct WorkspaceConfig
        {
            std::string Name;
            ArchitectureType Architecture;
            std::filesystem::path FilePath;

            std::vector<std::string> Configurations;
            std::vector<std::string> Defines;
        };

        struct FilterConfig
        {
            std::string FilterParameter;

            std::vector<std::string> Defines;
            std::vector<std::string> Links;
        };

        struct ProjectConfig
        {
            std::string Name;
            std::string Dialect;

            LanguageType Language;
            KindType Kind;

            std::string PrecompiledHeader;

            std::vector<std::string> Files;
            std::vector<std::string> IncludedDirectories;
            std::vector<std::string> Links;
            std::vector<std::string> Defines;

            std::vector<FilterConfig> Filters;
        };

        struct Projects
        {
            std::vector<ProjectConfig> InlineProjects;
            std::vector<ProjectConfig> ExternalProjects;
        };
    public:
        NewCMakePlatform();
    private:
        std::string ParseWorkspace(size_t* outPos);
        std::string ParseProject(size_t* outPos);

        void GenerateWorkspaceConfig();
        std::vector<ProjectConfig> GenerateProjectConfigs();
    };
}
