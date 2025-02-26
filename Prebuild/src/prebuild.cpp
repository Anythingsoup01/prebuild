#include <iostream>
#include "Core/Utils.h"
#include "Core/Platform.h"


int main(int argc, char** argv)
{
    Utils::System system = Utils::GetSystem(argv[1]);

    switch (argc)
    {
        case 1:
        {
            Utils::PrintError("Not enough arguments provided!");
            break;
        }
        case 2:
        {
            Utils::PrintError("Not enough arguments provided! Please provide a version!");
            break;
        }
        case 3:
        {
            std::string version(argv[2]);
            Prebuild::Platform::Create(system, version);
            break;
        }
        default:
        {
            std::cout << "ERROR: Too many arguments provided!" << std::endl;
            break;
        }
    }
}
