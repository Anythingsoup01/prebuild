#pragma once

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
