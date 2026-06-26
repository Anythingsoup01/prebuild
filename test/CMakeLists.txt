add_executable(Test-Proj)

set_property(TARGET Test-Proj PROPERTY CXX_STANDARD 20)
target_sources(Test-Proj PRIVATE
src/test_recurse/recurse.cpp
src/main.cpp
)

target_include_directories(Test-Proj PRIVATE
include
)



add_subdirectory(Test-External)


add_subdirectory(Test-Multi-Project)


