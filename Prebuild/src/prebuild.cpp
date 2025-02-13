#include <iostream>
#include "Core/Platform.h"
int main(int argc, char** argv)
{
    switch (argc)
    {
        case 1:
        {
            std::cout << "ERROR: not enough arguments provided!" << std::endl;
            break;
        }
        case 2:
        {
            Prebuild::Ref<Prebuild::Platform> platform = Prebuild::Platform::Create(argv[1]);
            break;
        }
        default:
        {
            std::cout << "ERROR: Too many arguments provided!" << std::endl;
            break;
        }
    }
}