cmake_minimum_required(VERSION 3.11)

include(FetchContent)

FetchContent_Declare(
    nanopb
    GIT_REPOSITORY https://github.com/nanopb/nanopb.git
    GIT_TAG        0.4.9
)

FetchContent_MakeAvailable(nanopb)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${nanopb_SOURCE_DIR}/extra)

find_package(Python3 REQUIRED)
find_package(Nanopb REQUIRED)
