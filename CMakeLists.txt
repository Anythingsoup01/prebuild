cmake_minimum_required(VERSION 3.30.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(Prebuild LANGUAGES C CXX)
set(GDEFINES
)

set(SRCS
    Prebuild/src/Core/Platform.cpp
    Prebuild/src/Core/Utils.cpp
    Prebuild/src/prebuild.cpp
    Prebuild/src/Platform/CMake/CMakePlatform.cpp
)
add_executable(prebuild ${SRCS})
target_include_directories(prebuild PRIVATE
    Prebuild/src
)
