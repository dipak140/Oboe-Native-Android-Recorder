cmake_minimum_required(VERSION 3.22.1)
project("oboenative")

get_filename_component(ROOT_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/../../.. ABSOLUTE)

add_library(${CMAKE_PROJECT_NAME} SHARED
        native-lib.cpp
)


target_link_libraries(${CMAKE_PROJECT_NAME}
        android
        log)