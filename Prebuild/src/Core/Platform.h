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

        enum KindType
        {
            NONE = 0,
            STATICLIB = 1,
            DYNAMICLIB = 2,
            CONSOLEAPP = 3,
        };

        const char* Keywords[10] = 
        {
            // WORKSPACE
            "workspace",
            "version",
            "architecture",
            "flags",
            // PROJECTS
            "project",
            "mainfile",
            "kind",
            "files",
            "includedirs",
            "links"
        };
    };
}