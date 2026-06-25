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
  std::string BuildProjectFiles(const std::string &projName, const std::vector<std::filesystem::path>& files, const std::filesystem::path &relPath);
  std::string BuildProjectKind(const std::string &projName, const KindType &projKind);
  std::string BuildProjectLanguage(const std::string &projName, const LanguageType &projLang, const std::string &projDialect);
  std::string BuildProjectIncludeDirs(const std::string &projName, const std::vector<std::filesystem::path> &projIncludeDirs);
  std::string BuildProjectLinks(const std::string &projName, const std::vector<std::string> projLinks);
  std::string BuildProjectFlags(const std::string &projName, const std::vector<std::string> projFlags);
  std::string BuildProjectDefines(const std::string &projName, const std::vector<std::string> projDefines);
  
  std::string BuildProject(const ProjectConfig &proj, const std::filesystem::path &relPath);

  std::string StartFilterSystem(const FilterConfig &filter);
  std::string StartFilterConfiguration(const FilterConfig &filter);

  std::string BuildFilter(const FilterConfig &cfg);

  std::string GetCMakeSyntax(const std::string &keyword);
private:
  std::string m_Version;
  WorkspaceConfig m_WorkspaceConfig;
  std::vector<ProjectFileConfig> m_FileConfigs;
};
} // namespace Prebuild
