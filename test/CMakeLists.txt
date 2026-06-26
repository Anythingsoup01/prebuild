cmake_minimum_required(VERSION 4.3.4)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(Test-Wks LANGUAGES C CXX)
set(GDEFINES
    Test-Wks-Define
    Test123
)

add_executable(Test-Proj)

set_property(TARGET Test-Proj PROPERTY CXX_STANDARD 20)
target_sources(Test-Proj PRIVATE
src/test_recurse/recurse.cpp
src/main.cpp
)

target_include_directories(Test-Proj PRIVATE
include
)

target_compile_definitions(Test-Proj PUBLIC GDEFINES)
if(CMAKE_BUILD_TYPE STREQUAL Debug)

target_compile_definitions(Test-Proj PUBLIC
test_config_define
)

endif()

add_subdirectory(Test-External)

add_subdirectory(Test-Multi-Project)

