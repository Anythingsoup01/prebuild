workspace = "Test"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release"
    }

project "Daddy"
    kind "StaticLib"

project "Mommy"
    kind "DynamicLib"

external "test"
