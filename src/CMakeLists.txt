cmake_minimum_required(VERSION 3.22)

add_executable(
    ${PROJECT_NAME}
    main.c
)


add_subdirectory(bsp)

include(Nanopb)
nanopb_generate_cpp(
    TARGET lib-protocols
    RELPATH protocols/protobuf
    protocols/protobuf/raptor/v1/battery.proto
    protocols/protobuf/raptor/v1/bldc.proto
    protocols/protobuf/raptor/v1/env.proto
    protocols/protobuf/raptor/v1/metadata.proto
    protocols/protobuf/raptor/v1/raptor.proto
    protocols/protobuf/raptor/v1/relay.proto
)

# create third party libraries
cubeh7_add_lwip(common/config)
cubeh7_add_cmsis(lib-freertos)
cubeh7_add_freertos()
cubeh7_add_hal()

add_library(lib-freertos STATIC ${FREERTOS_SRCS})
target_include_directories(lib-freertos PUBLIC ${FREERTOS_INC} common/config)
target_link_libraries(lib-freertos ${LIB_BSP})
add_library(lib-lwip STATIC ${LWIP_SRCS})
target_link_libraries(lib-lwip lib-freertos)

add_subdirectory(os)
add_subdirectory(common)

target_compile_options(
    ${LIB_RAPTOR}
    PUBLIC -Wno-unused-parameter -Wpedantic -fno-builtin -Wall -Wextra -ffunction-sections -fdata-sections -fomit-frame-pointer
    PUBLIC $<$<CONFIG:Debug>:-DRAPTOR_DEBUG>
)

target_link_libraries(
    ${LIB_RAPTOR}
    ${LIB_BSP}
    lib-freertos
    lib-lwip
    lib-protocols
)


target_link_libraries(${PROJECT_NAME} ${LIB_BSP} ${LIB_RAPTOR} lib-freertos lib-lwip)


