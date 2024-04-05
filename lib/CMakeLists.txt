# build raptor core lib

add_library(
  ${RAPTOR_LIB}
  STATIC
  sysreg.c
)
target_include_directories(${RAPTOR_LIB} PUBLIC .)
