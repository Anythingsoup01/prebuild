#pragma once
#include "Core/Platform.h"

#include <string>
#include <vector>
#include <unordered_map>

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
        ProjectConfig BuildProjectConfig(int& index);

        bool ContainsKeyword(std::string& line);

        std::string ParseSingleResponse(const char* keyword, std::string& line);
        
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
            KindType Kind;

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