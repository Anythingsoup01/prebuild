cmake_minimum_required(VERSION 3.16)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Prebuild)

set(Files
Prebuild/src/Core/Utils.cpp
Prebuild/src/Core/Platform.cpp
Prebuild/src/Platform/CMake/CMakePlatform.cpp
)

add_executable(prebuild Prebuild/src/prebuild.cpp ${Files})

set(Dirs
Prebuild/src
)

target_include_directories(prebuild PUBLIC ${Dirs})

