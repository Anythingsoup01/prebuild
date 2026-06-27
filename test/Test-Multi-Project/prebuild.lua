Project = {
  name = "Test-Mutli-1",
  kind = "StaticLib",
  language = "C++",
  dialect = "20",

  files = {
    "src/*.cpp",
  },

  includedirs = {
    "include",
  },
}

Project = {
  name = "Test-Multi-2",
  kind = "SharedLib",
  language = "C++",
  dialect = "20",

  files = {
    "src/**.cpp",
  },

  includedirs = {
    "include",
  },
}
