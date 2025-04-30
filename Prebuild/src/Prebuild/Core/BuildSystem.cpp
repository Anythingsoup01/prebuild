#include "pbpch.h"
#include "BuildSystem.h"

#include "Prebuild/Platform/CMake/CMakeBuildSystem.h"

Scope<BuildSystem> BuildSystem::Create(char** arguments)
{
    System currentSystem = GetSystem(arguments[1]);

    switch (currentSystem) 
    {
        case System::CMake:
        {
            return CreateScope<CMakeBuildSystem>(arguments[2]);
        }
        case System::Possum:
        {
            break;
        }
        default:
            break;
    }
    PB_CRITICAL("Build System not supported!");
    return nullptr;
}

BuildSystem::System BuildSystem::GetSystem(char* argument)
{
    std::string str(argument);
    if (str == "cmake")
        return System::CMake;
    if (str == "psm")
        return System::Possum;

    return System::None;

}
