cmake_minimum_required(VERSION 4.3.4)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(Prebuild LANGUAGES C CXX)
add_executable(prebuild)

set_property(TARGET prebuild PROPERTY CXX_STANDARD 20)
target_sources(prebuild PRIVATE
Prebuild/src/prebuild.cpp
Prebuild/src/Platform/CMake/CMakePlatform.cpp
Prebuild/src/Core/Platform.cpp
Prebuild/src/Core/Utils.cpp
)

target_precompile_headers(prebuild PUBLIC Prebuild/src/pbpch.h)

target_include_directories(prebuild PRIVATE
Prebuild/src
)

target_link_libraries(prebuild
lua5.4
)


add_subdirectory(test)

