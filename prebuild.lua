Workspace = {
    name = "Prebuild",
}

Project = {
    name = "prebuild",
    language = "C++",
    dialect = "20",
    kind = "ConsoleApp",

    pch = "Prebuild/src/pbpch.h",

    files = {
        "Prebuild/src/*.cpp",
        "Prebuild/src/*.h",
    },

    includedirs = {
        "Prebuild/src"
    },

    links = {
        "lua5.4",
    },
}
