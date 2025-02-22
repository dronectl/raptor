target_sources(
  ${LIB_RAPTOR}
  PRIVATE
  uassert.c
  cbuffer.c
  logger.c
  sysreg.c
  registers.c
  dtc.c
)

target_include_directories(${LIB_RAPTOR} PUBLIC .)
