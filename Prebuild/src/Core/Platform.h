#pragma once
#include "Core.h"

namespace Prebuild
{
    class Platform
    {
        public:
        Platform() = default;
        ~Platform() = default;

        static Scope<Platform> Create(const char* platformType);
    };
}