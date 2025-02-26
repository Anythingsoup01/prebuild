workspace "Prebuild"

project "prebuild"
    language "C++"
    dialect "17"
    kind "ConsoleApp"

    files
    {
        "Prebuild/src/Core/Platform.cpp",
        "Prebuild/src/Core/Utils.cpp",
        "Prebuild/src/prebuild.cpp",
        "Prebuild/src/Platform/CMake/CMakePlatform.cpp",
    }

    includedirs
    {
        "Prebuild/src"
    }

