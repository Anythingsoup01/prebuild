workspace "Example"
    version "3.16"
    architecture "x86_64"

    flags
    {
        "VSCODE",
    }

project "EXAMPLEPROJECT"
    mainfile "EXAMPLEPROJECT/src/main.cpp"
    kind "ConsoleApp"

    files
    {
        "EXAMPLEPROJECT/src/Core/Print.cpp",
    }

    includedirs
    {
        "EXAMPLEPROJECT/src",
    }

    links
    {
        "EXAMPLELIBRARY",
    }