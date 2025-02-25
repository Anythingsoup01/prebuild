#pragma once
#include "Core.h"

#include <string>

namespace Prebuild
{
    class Platform
    {
        public:
        Platform() = default;
        ~Platform() = default;

        static Scope<Platform> Create(const char* platformType);

        const size_t NPOS = std::string::npos;

        enum ProjectType
        {
            pNONE    = 0,
            INLINE   = 1,
            EXTERNAL = 2,
        };

        enum KindType
        {
            kNONE      = 0,
            STATICLIB  = 1,
            SHAREDLIB  = 2,
            CONSOLEAPP = 3,
        };

        enum LanguageType
        {
            lNONE = 0,
            C     = 1,
            CXX   = 2,
        };

        enum ArchitectureType
        {
            aNONE = 0,
            X86 = 1,
            X64 = 2,
        };

        const char* Keywords[10] = 
        {
            // WORKSPACE
            "workspace",
            "architecture",
            "configurations",
            "defines",
            // PROJECTS
            "project",
            "kind",
            "files",
            "includedirs",
            "links",
            "configurations",
        };

        const char* PathKeywords[1] = 
        {
            "$(ROOTDIR)",
        };
    };
}
