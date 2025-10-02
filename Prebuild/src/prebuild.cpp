#include <iostream>
#include "Core/Platform.h"
#include "Core/Utils.h"


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
        case 3:
        {
            std::filesystem::path currentPath = std::filesystem::current_path();
            if (argc == 3)
            {
                currentPath /= argv[2];
            }
            Prebuild::Platform platform(currentPath);
            break;
        }
        default:
        {
            std::cout << "ERROR: Too many arguments provided!" << std::endl;
            break;
        }
    }
}
