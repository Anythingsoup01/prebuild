cmake_minimum_required(VERSION 4.3.3)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(Test-Wks LANGUAGES C CXX)
set(GDEFINES
    Test-Wks-Define
    Test123
)

add_executable(Test-Proj)

set_property(TARGET Test-Proj PROPERTY CXX_STANDARD 20)
target_sources(Test-Proj PRIVATE
src/main.cpp
src/test_recurse/recurse.cpp
)

target_include_directories(Test-Proj PRIVATE
"include"
)

target_compile_definitions(Test-Proj PUBLIC GDEFINES)

add_subdirectory(Test-External)

add_subdirectory(Test-Multi-Project)

