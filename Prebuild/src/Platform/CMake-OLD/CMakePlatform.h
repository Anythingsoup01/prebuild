#pragma once
#include "Core/Platform.h"

#include <string>
#include <vector>


namespace Prebuild
{
    class CMakePlatform : public Platform
    {
        struct ProjectConfig;
    public:
        CMakePlatform();
        std::string ParseWorkspace(size_t& pos);
        std::string ParseProject(size_t& pos, std::string dir = "");

        void BuildWorkspaceConfig();
        ProjectConfig BuildProjectConfig(std::string& strCache);

        bool ContainsKeyword(std::string& line, std::string& outKeyword);
        bool ContainsPathKeyword(std::string& line, std::string& outKeyword);

        std::string GetCMakeSyntax(std::string& keyword);

        std::string ParseSingleResponse(const char* keyword, std::string& line);
        std::vector<std::string> ParseMultipleResponse(const char* keyword, std::string& strCache, size_t& pos);

        void Build();
        std::string BuildWorkspace();
        std::string BuildProject(ProjectConfig& cfg);
        
    private:
        struct WorkspaceConfig
        {
            std::string Name;
            std::string Version;
            std::string Architecture;
            
            std::string Configuration;
            std::vector<std::string> Flags;
        };
        WorkspaceConfig m_WorkspaceConfig;

        struct ProjectConfig
        {
            std::string Name;
            std::string MainFileDirectory;
            std::string Kind;

            std::vector<std::string> Files;
            std::vector<std::string> IncludeDirectories;
            std::vector<std::string> Links;
        };
        struct Projects
        {
            std::vector<ProjectConfig> InlineProjects;
            std::vector<ProjectConfig> ExternalProjects;
        };
        Projects m_Projects;
    private:
        std::string m_PrebuildString;
        std::string m_WorkspaceString;
        std::vector<std::string> m_InlineProjectStrings;
        std::vector<std::string> m_ExternalProjectStrings;
        std::vector<std::string> m_ExternalProjectDirs;

    };
}
