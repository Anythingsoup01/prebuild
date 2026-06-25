Workspace = {
  name = "Test-Wks",
  architecture = "x64",
  defines = {
    "Test-Wks-Define",
    "Test123",
  },
}

Project = {
  name = "Test-Proj",
  kind = "ConsoleApp",
  language = "C++",
  dialect = "20",

  files = {
    "src/**.cpp",
  },

  includedirs = {
    "include",
  },
}

External = "Test-External"
External = "Test-Multi-Project"
