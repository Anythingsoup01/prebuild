cmake_minimum_required(VERSION 3.16)

set(SRCS 
    ${CMAKE_SOURCE_DIR}/EXAMPLELIBRARY/src/Core/Print.cpp

)
add_executable(EXAMPLEPROJECT ${SRCS})
target_include_directories(EXAMPLEPROJECT PRIVATE 
    ${CMAKE_SOURCE_DIR}/EXAMPLELIBRARY/src/Core/Print.cpp

)

