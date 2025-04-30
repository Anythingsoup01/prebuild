#include "pbpch.h"
#include "CMakeBuildSystem.h"

#include <fstream>

CMakeBuildSystem::CMakeBuildSystem(const std::string& version)
{
    if (ParseRootFile() != PBSuccess) return;
}

PBResult CMakeBuildSystem::ParseRootFile()
{
    std::ifstream in("prebuild.lua");
    if (!in.is_open())
    {
        PB_ERROR("Could not locate prebuild.lua file!");
        return PBError;
    }

    return PBSuccess;
}




