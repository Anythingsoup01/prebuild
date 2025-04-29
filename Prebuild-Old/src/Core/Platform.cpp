#include "Platform.h"
#include "Platform/CMake/CMakePlatform.h"

namespace Prebuild
{
    Scope<Platform> Platform::Create(Utils::System system,const std::string& version)
    {
        switch (system)
        {
            case Utils::System::CMAKE:
            {
                return CreateScope<CMakePlatform>(version);
            }
            default:
                return nullptr;

        }

    }
}
