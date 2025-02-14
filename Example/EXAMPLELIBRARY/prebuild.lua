project "EXAMPLELIBRARY"
    mainfile "src/main.cpp"
    kind "StaticLib"

    files
    {
        "src/Core/Print.cpp",
    }

    includedirs
    {
        "src",
    }