cmake_minimum_required(VERSION 3.22)
function (find_openocd)
  # Find the host's openocd program
  message(STATUS "Finding OpenOCD binary")
  find_program(
    OPENOCD_EXECUTABLE
    NAMES openocd
    PATHS "/usr/local/bin"
          "/usr/bin"
          "$ENV{PROGRAMFILES}/OpenOCD"
    DOC "Path to the openocd program"
  )
  # Check if the program was found
  if (NOT OPENOCD_EXECUTABLE)
    message(WARNING "Could not find openocd program")
  endif()
endfunction()
