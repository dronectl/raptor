# build raptor core lib
message(STATUS "Building raptor lib")
set(RAPTOR_LIB libraptor)
add_library(
  ${RAPTOR_LIB}
  STATIC
  ${PROJECT_SOURCE_DIR}/lib/sysreg.c
)
target_include_directories(${RAPTOR_LIB} PUBLIC ${PROJECT_SOURCE_DIR}/lib)
target_compile_options(
  ${RAPTOR_LIB}
  PUBLIC -Wall -Wpedantic -Wextra -ffunction-sections -fdata-sections
  PUBLIC $<$<CONFIG:Debug>:-DRAPTOR_DEBUG -O0 -g>
  PUBLIC $<$<CONFIG:Release>:-O3>
)
