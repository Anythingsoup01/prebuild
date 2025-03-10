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

        enum class ProjectType
        {
            NONE = 0,
            INLINE,
            EXTERNAL,
        };

        enum class KindType
        {
            NONE = 0,
            STATICLIB,
            SHAREDLIB,
            CONSOLEAPP,
        };

        enum class LanguageType
        {
            NONE = 0,
            C,
            CXX,
        };

        enum class ArchitectureType
        {
            NONE = 0,
            X86,
            X64,
        };

        enum class KeywordType
        {
            NONE = 0,
            WORKSPACE,
            PROJECT,
            FILTER,
            FILEPATH,
        };

        const char* WorkspaceKeywords[3] =
        {
            "workspace",
            "architecture",
            "defines",
        };

        const char* ProjectKeywords[9]
        {
            "project",
            "language",
            "dialect",
            "kind",
            "pch",
            "files",
            "includedirs",
            "links",
            "filter",
        };

        const char* FilterKeywords[2]
        {
            "filter",
            "defines",
        };

        const char* AllKeywords[12]
        {
            "workspace",
            "architecture",
            "defines",
            "project",
            "language",
            "dialect",
            "kind",
            "pch",
            "files",
            "includedirs",
            "links",
            "filter",
        };

        const char* PathKeywords[1] =
        {
            "$(WORKSPACEDIR)",
        };
    };
}
