cmake_minimum_required(VERSION 3.22.1)
project("oboenativemodule")

get_filename_component(ROOT_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/../../.. ABSOLUTE)

set (OBOE_DIR ${ROOT_DIR}/oboe)
add_subdirectory(${OBOE_DIR} ./oboe-bin)

add_library(${CMAKE_PROJECT_NAME} SHARED
        LiveEffectEngine.cpp
        oboenativemodule.cpp
)
target_link_libraries(${CMAKE_PROJECT_NAME}
        oboe
        android
        log)