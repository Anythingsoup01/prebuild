#pragma once

#include "Core.h"

class BuildSystem
{
public:
    static Scope<BuildSystem> Create(char** arguments);

private:
    enum class System
    {
        None = 0,
        CMake,
        Possum,
    };

    static System GetSystem(char* argument);
};

enum PBResult
{
    None = 0,
    PBSuccess = 1,
    PBError = 2
};
