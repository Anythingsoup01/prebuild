add_executable(Test-Mutli-1)

set_property(TARGET Test-Mutli-1 PROPERTY CXX_STANDARD 20)
target_sources(Test-Mutli-1 PRIVATE
src/multi.cpp
)

target_include_directories(Test-Mutli-1 PRIVATE
"include"
)

target_compile_definitions(Test-Mutli-1 PUBLIC GDEFINES)


add_executable(Test-Multi-2)

set_property(TARGET Test-Multi-2 PROPERTY CXX_STANDARD 20)
target_sources(Test-Multi-2 PRIVATE
src/multi.cpp
)

target_include_directories(Test-Multi-2 PRIVATE
"include"
)

target_compile_definitions(Test-Multi-2 PUBLIC GDEFINES)


