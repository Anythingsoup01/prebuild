#include "Platform.h"
#include "Platform/CMake/CMakePlatform.h"

namespace Prebuild
{
    Scope<Platform> Platform::Create(const char* platformType)
    {
        std::string platformString(platformType);
        if (platformString == "cmake")
        {
            return CreateScope<CMakePlatform>();
        }
    }
}