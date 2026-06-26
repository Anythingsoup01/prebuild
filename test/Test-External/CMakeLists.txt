add_library(Test-Ext)

set_property(TARGET Test-Ext PROPERTY C_STANDARD 99)
target_sources(Test-Ext PRIVATE
src/test_single.c
src/multiple/test_multiple.c
)

target_include_directories(Test-Ext PRIVATE
include
)

target_compile_definitions(Test-Ext PUBLIC
TEST_DEFINES
)

target_compile_definitions(Test-Ext PUBLIC GDEFINES)


