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
        CMakePlatform(const std::string& version);
    private:

        std::string ParseWorkspace(size_t& outPos);
        std::string ParseProject(size_t& outPos, std::string dir = "");

        void BuildWorkspaceConfig();
        ProjectConfig BuildProjectConfig(const std::string& strCache);

        void Build();
        std::string BuildProject(ProjectConfig& cfg);

        // Utility
        bool CheckSyntax(const std::string& strCache);
        std::string GetKeyword(const std::string& line);
        bool IsMultiParameter(const std::string& keyword);
        bool ContainsKeyword(const std::string& line, std::string& outKeyword, bool isFilePath = false);
        bool IsSetForMultipleParameters(const std::string& strCache, size_t& pos);

        std::string ParseField(const std::string& line, const std::string& keyword);
        std::vector<std::string> ParseMultipleFields(const std::string& strCache, size_t& outPos, const std::string& keyword);

        ArchitectureType StringToArchitectureType(const std::string archStr);
        LanguageType StringToLanguageType(std::string langStr);
        KindType StringToKindType(std::string kindStr);

        ProjectType CheckProjectType(const std::string& line);

    private:
        std::string m_Version;
        std::string m_RootPrebuildString;
        std::string m_WorkspaceString;
        std::vector<std::string> m_NonPrebuildProject;
        std::vector<std::string> m_InlineProjectStrings;
        std::unordered_map<std::string, std::string> m_ExternalProjectStrings;

        WorkspaceConfig m_WorkspaceConfig;

        Projects m_Projects;

    };
}
