cmake_minimum_required(VERSION 2.8)

project(simple-log-library)

include_directories(../buffer ../log ../roll_file ../timestamp)

aux_source_directory(../buffer SRC_LIST1)
aux_source_directory(../log SRC_LIST2)
aux_source_directory(../roll_file SRC_LIST3)
aux_source_directory(../timestamp SRC_LIST4)

set(CMAKE_EXE_LINKER_FLAGS "-pthread")
add_compile_options(-w -Wall -std=c++11)

add_executable(../main ../main.cpp ${SRC_LIST1} ${SRC_LIST2} ${SRC_LIST3} ${SRC_LIST4})
