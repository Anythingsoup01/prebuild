#include "pbpch.h"

#include "BuildSystem.h"

int main(int argc, char** argv)
{
    Log::Init();

    switch (argc)
    {
        case 1:
        case 2:
        {
            PB_ERROR("Not enough arguments provided!");
            break;
        }
        case 3:
        {
            Scope<BuildSystem> system;
            system = BuildSystem::Create(argv);
            return system == nullptr ? -1 : 0;
            break;
        }
        default:
        {
            PB_ERROR("Too many arguments provided!");
            break;
        }
    }
    return 0;
}
