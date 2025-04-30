cmake_minimum_required(VERSION 3.30)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(Prebuild LANGUAGES CXX)
set(GDEFINES
)

set(SRCS
    Prebuild/src/Prebuild/Platform/CMake/CMakeBuildSystem.cpp
    Prebuild/src/Prebuild/Core/BuildSystem.cpp
    Prebuild/src/Prebuild/Core/Log.cpp
    Prebuild/src/Prebuild/Core/Prebuild.cpp
    Prebuild/src/Prebuild/Platform/CMake/CMakeBuildSystem.h
    Prebuild/src/Prebuild/Core/Log.h
    Prebuild/src/Prebuild/Core/BuildSystem.h
    Prebuild/src/Prebuild/Core/Core.h
    Prebuild/src/pbpch.h
)
add_executable(prebuild ${SRCS})
set_property(TARGET prebuild PROPERTY CXX_STANDARD 20)

target_include_directories(prebuild PRIVATE
    Prebuild/src
    vendor/spdlog/include
)

