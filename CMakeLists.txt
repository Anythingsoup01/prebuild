cmake_minimum_required(VERSION 3.30)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(Prebuild LANGUAGES CXX)
set(SRCS
    Prebuild/src/prebuild.cpp
    Prebuild/src/Core/Platform.cpp
    Prebuild/src/Core/Utils.cpp
    Prebuild/src/pbpch.h
    Prebuild/src/Core/Platform.h
    Prebuild/src/Core/Utils.h
    Prebuild/src/Core/Core.h
)
add_executable(prebuild ${SRCS})
set_property(TARGET prebuild PROPERTY CXX_STANDARD 20)

target_precompile_headers(prebuild PUBLIC Prebuild/src/pbpch.h)
target_include_directories(prebuild PRIVATE
    Prebuild/src
)

target_link_libraries(prebuild
    lua
)

