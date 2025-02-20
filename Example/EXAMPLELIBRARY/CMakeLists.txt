cmake_minimum_required(VERSION << 3.16)

set(SRCS 
    src/Core/Print.cpp
)
add_library(EXAMPLELIBRARY STATIC ${SRCS})
target_include_directories(EXAMPLELIBRARY PRIVATE 
    src/Core/Print.cpp
)

