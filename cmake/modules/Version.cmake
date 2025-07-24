# 
# Version.cmake
#
# This file is used to configure the version header file for the project.
# 

function(version_file_configure VERSION_MAJOR VERSION_MINOR VERSION_PATCH)
    set(VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/version.h")
    message(STATUS "Generating ${VERSION_FILE} header for version ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.h.in" "${VERSION_FILE}" @ONLY)
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
endfunction()

