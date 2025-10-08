set(SRCS
    "${CMAKE_SOURCE_DIR}/Daddy/src/daddy.cpp"
)
add_library(Daddy STATIC ${SRCS})
add_subdirectory(Child)
