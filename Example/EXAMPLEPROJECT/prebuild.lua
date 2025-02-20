project "EXAMPLEPROJECT"
    mainfile "src/main.cpp"
    kind "ConsoleApp"

    files
    {
        "$(ROOTDIR)EXAMPLELIBRARY/src/Core/Print.cpp",
    }

    includedirs
    {
        "src",
        "$(ROOTDIR)EXAMPLELIBRARY/src",
    }

    links
    {
        "EXAMPLELIBRARY",
    }