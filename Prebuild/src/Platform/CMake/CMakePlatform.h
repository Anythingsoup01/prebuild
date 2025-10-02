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

    public:
        CMakePlatform(WorkspaceConfig& workspaceConfig, std::vector<ProjectConfig>& projects);
    private:
        void Build();
        std::string BuildProject(ProjectConfig& cfg);
        std::string BuildFilter(const FilterConfig& cfg, const std::string& target);
        std::string BuildFilterPlatform(const FilterConfig& cfg, const std::string& target);
        std::string BuildFilterConfigurations(const FilterConfig& cfg, const std::string& target);

        std::string GetCMakeSyntax(const std::string& keyword);

    private:
        std::string m_Version;
        std::string m_RootPrebuildString;
        std::string m_WorkspaceString;
        std::vector<std::string> m_NonPrebuildProject;
        std::vector<std::string> m_InlineProjectStrings;
        std::unordered_map<std::string, std::string> m_ExternalProjectStrings;
    };
}
