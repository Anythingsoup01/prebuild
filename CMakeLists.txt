cmake_minimum_required(VERSION 3.30)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(Prebuild LANGUAGES CXX)
set(GDEFINES
)

set(SRCS
    Prebuild/src/Platform/CMake/CMakePlatform.cpp
    Prebuild/src/Core/Utils.cpp
    Prebuild/src/Core/Platform.cpp
    Prebuild/src/prebuild.cpp
    Prebuild/src/Platform/CMake/CMakePlatform.h
    Prebuild/src/Core/Utils.h
    Prebuild/src/Core/Platform.h
    Prebuild/src/Core/Core.h
)
add_executable(prebuild ${SRCS})
set_property(TARGET prebuild PROPERTY CXX_STANDARD 17)

target_include_directories(prebuild PRIVATE
    Prebuild/src
)

