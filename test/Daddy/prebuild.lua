Project = {
  name = "Daddy",
  kind = "StaticLib",
  files = {
    "${WORKSPACEDIR}Daddy/src/*.cpp",
  },
}

Project = {
  name = "Mommy",
  kind = "StaticLib",
  files = {
    "${WORKSPACEDIR}Daddy/src/*.cpp",
  },
}

External = "Child"
