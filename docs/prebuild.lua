// Note - Comments do not work at the moment
workspace "docs-example"

project "doc-inline-example"
    language "C++"
    dialect "17"
    kind "ConsoleApp"

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
