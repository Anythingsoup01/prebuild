project "external-folder"
    language "C++"    // C++ C
    dialect "17"      // Depending on the language dialect will set c/c++ standard
    kind "ConsoleApp" // ConsoleApp StaticLib SharedLib

    includedirs
    {
        "doc/example/include1",
        "doc/example/include2",
        "doc/example/include3",
    }

    files
    {
        "doc/example/file1.cpp",
        "doc/example/file2.cpp",
        "doc/example/file3.cpp",
    }

    links
    {
        "ExampleLib1",
        "ExampleLib2",
        "ExampleLib3"
    }
