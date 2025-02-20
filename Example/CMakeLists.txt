cmake_minimum_required(VERSION 3.16)

project(Example)
set(ARCH x86_64)

set(FLAGS 
    -fVSCODE
    -fLINUX
)

add_subdirectory(EXAMPLELIBRARY)
add_subdirectory(EXAMPLEPROJECT)

