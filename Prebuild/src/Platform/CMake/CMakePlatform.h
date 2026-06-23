#pragma once

#include "Core/Platform.h"

namespace Prebuild {
class CMakePlatform : public Platform {
private:
public:
  CMakePlatform(const WorkspaceConfig &workspaceConfig,
                const std::vector<ProjectFileConfig> &fileConfigs);

private:
  void Build();
  std::string BuildProject(const ProjectConfig &proj);
  std::string BuildFilter(const FilterConfig &cfg, const std::string &target);
  std::string BuildFilterPlatform(const FilterConfig &cfg,
                                  const std::string &target);
  std::string BuildFilterConfigurations(const FilterConfig &cfg,
                                        const std::string &target);

  std::string GetCMakeSyntax(const std::string &keyword);

private:
  std::string m_Version;
  WorkspaceConfig m_WorkspaceConfig;
  std::vector<ProjectFileConfig> m_FileConfigs;
};
} // namespace Prebuild
