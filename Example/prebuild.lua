workspace "Example"
    version "3.16"
    architecture "x86_64"

    flags
    {
        "VSCODE",
        "LINUX",
    }

external("EXAMPLELIBRARY")
external("EXAMPLEPROJECT")

