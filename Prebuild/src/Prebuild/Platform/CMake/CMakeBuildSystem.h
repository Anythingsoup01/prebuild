#pragma once

#include "Prebuild/Core/BuildSystem.h"

class CMakeBuildSystem : public BuildSystem
{
public:
    CMakeBuildSystem(const std::string& version);
private:
    PBResult ParseRootFile();

    void ParseWorkspaceString();
    void ParseProjectString();
private:
    std::string m_RootFileString;
    std::string m_WorkspaceString;
};
