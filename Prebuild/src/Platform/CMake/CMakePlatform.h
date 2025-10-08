#pragma once


#include "Core/Platform.h"

namespace Prebuild
{
    class CMakePlatform : public Platform
    {
    private:

    public:
        CMakePlatform(const WorkspaceConfig& workspaceConfig, const std::vector<ProjectConfig>& projects);
    private:
        void Build();
        std::string BuildProject(const ProjectConfig& cfg);
        std::string BuildFilter(const FilterConfig& cfg, const std::string& target);
        std::string BuildFilterPlatform(const FilterConfig& cfg, const std::string& target);
        std::string BuildFilterConfigurations(const FilterConfig& cfg, const std::string& target);

        std::string GetCMakeSyntax(const std::string& keyword);

    private:
        std::string m_Version;
        WorkspaceConfig m_WorkspaceConfig;
        std::vector<ProjectConfig> m_Projects;
    };
}
