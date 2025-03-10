#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

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

        struct FilterConfig
        {
            std::string FilterParameter;

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
            std::vector<std::string> Defines;

            std::vector<FilterConfig> Filters;
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
        FilterConfig ParseFilter(const std::string& strCache, size_t& outPos, const std::string& keyword);

        void BuildWorkspaceConfig();
        ProjectConfig BuildProjectConfig(const std::string& strCache, bool isExternal);

        void Build();
        std::string BuildProject(ProjectConfig& cfg);
        std::string BuildFilter(FilterConfig& cfg, const std::string& target);

        // Utility
        bool CheckSyntax(const std::string& strCache);
        std::string GetKeyword(const std::string& line);
        bool IsMultiParameter(const std::string& keyword);
        bool ContainsKeyword(const std::string& line, std::string& outKeyword, const KeywordType type);
        bool IsSetForMultipleParameters(const std::string& strCache, size_t& pos);

        std::string ParseField(const std::string& line, const std::string& keyword);
        std::vector<std::string> ParseMultipleFields(const std::string& strCache, size_t& outPos, const std::string& keyword);
        std::vector<std::string> GetAllFilesWithExtension(const std::string& line, const std::string& extension, const std::string folderName = "");
        std::vector<std::string> SearchDirectoryFor(const std::filesystem::path filePath, const std::string extension);

        ArchitectureType StringToArchitectureType(const std::string archStr);
        LanguageType StringToLanguageType(std::string langStr);
        KindType StringToKindType(std::string kindStr);

        ProjectType CheckProjectType(const std::string& line);

        void GetCMakeSyntax(const std::string& keyword,std::string& outLine);

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
