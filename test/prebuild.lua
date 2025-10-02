workspace "Test"
    architecture "x64"

    configurations
    {
        "Debug"
    }

project "Daddy"
    kind "StaticLib"

project "Mommy"
    kind "DynamicLib"

external "test"
