cmake_minimum_required(VERSION 3.28)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(Prebuild LANGUAGES CXX)
set(SRCS
    Prebuild/src/prebuild.cpp
    Prebuild/src/Core/Utils.cpp
    Prebuild/src/Core/Platform.cpp
    Prebuild/src/Core/Core.h
    Prebuild/src/Core/Utils.h
    Prebuild/src/Core/Platform.h
    Prebuild/src/pbpch.h
)
add_executable(prebuild ${SRCS})
set_property(TARGET prebuild PROPERTY CXX_STANDARD 20)

target_precompile_headers(prebuild PUBLIC Prebuild/src/pbpch.h)
target_include_directories(prebuild PRIVATE
    Prebuild/src
)

