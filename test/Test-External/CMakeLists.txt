add_executable(Test-Ext)

set_property(TARGET Test-Ext PROPERTY C_STANDARD 20)
target_sources(Test-Ext PRIVATE
src/test_single.c
src/multiple/test_multiple.c
)

target_include_directories(Test-Ext PRIVATE
"include"
)

target_compile_definitions(Test-Ext PUBLIC GDEFINES)


