workspace "Prebuild"

project "prebuild"
    language "C++"
    dialect "17"
    kind "ConsoleApp"

    files
    {
        "Prebuild/src/*.cpp",
        "Prebuild/src/*.h",
    }

    includedirs
    {
        "Prebuild/src"
    }

