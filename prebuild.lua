workspace "Prebuild"

project "prebuild"
    language "C++"
    dialect "17"
    kind "ConsoleApp"

    files
    {
        "Prebuild/src/Core/Core.h",
        "Prebuild/src/Core/Platform.cpp",
        "Prebuild/src/Core/Platform.h",
        "Prebuild/src/Core/Utils.cpp",
        "Prebuild/src/Core/Utils.h",
        "Prebuild/src/Platform/CMake/CMakePlatform.cpp",
        "Prebuild/src/Platform/CMake/CMakePlatform.h",
        "Prebuild/src/prebuild.cpp"
    }

    includedirs
    {
        "Prebuild/src"
    }

