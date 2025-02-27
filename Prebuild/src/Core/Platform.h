#pragma once
#include "Core.h"
#include "Core/Utils.h"

#include <string>

namespace Prebuild
{
    class Platform
    {
        public:
        Platform() = default;
        ~Platform() = default;

        static Scope<Platform> Create(Utils::System system, const std::string& version);

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
            "defines",
            // PROJECTS
            "project",
            "language",
            "dialect",
            "kind",
            "files",
            "includedirs",
            "links",
        };

        const char* PathKeywords[1] = 
        {
            "$(ROOTDIR)",
        };
    };
}
