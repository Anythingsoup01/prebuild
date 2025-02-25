#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#include "Core/Platform.h"

namespace Prebuild
{
    class CMakePlatform : public Platform
    {
    private:
        struct WorkspaceConfig
        {
            std::string Name;
            ArchitectureType Architecture;

            std::vector<std::string> Configurations;
            std::vector<std::string> Defines;
        };

        struct ProjectConfig
        {
            std::string Name;
            std::string Dialect;

            LanguageType Language;
            KindType Kind;

            std::vector<std::string> Files;
            std::vector<std::string> IncludedDirectories;
            std::vector<std::string> Links;
            std::unordered_map<std::string, std::vector<std::string>> ConfigurationDefines;
        };

        struct Projects
        {
            std::vector<ProjectConfig> InlineProjects;
            std::unordered_map<std::string, ProjectConfig> ExternalProjects;
        };
    public:
        CMakePlatform();
    private:

        std::string ParseWorkspace(size_t& outPos);
        std::string ParseProject(size_t& outPos, std::string dir = "");

        ProjectType CheckProjectType(const std::string& line);

        // Utility
        bool CheckSyntax(const std::string& strCache);
        std::string GetKeyword(const std::string& line);
        bool IsMultiParameter(const std::string& keyword);

    private:
        std::string m_RootPrebuildString;
        std::string m_WorkspaceString;
        std::vector<std::string> m_InlineProjectStrings;
        std::unordered_map<std::string, std::string> m_ExternalProjectStrings;

        Projects m_Projects;

    };
}
