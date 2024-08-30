cmake_minimum_required(VERSION 3.20)

project(
    rear-light
    DESCRIPTION "Rear Light"
    LANGUAGES C CXX ASM
    VERSION "1.0.0")

find_package(Python REQUIRED)

include(tools/cmake/CPM.cmake)

CPMAddPackage("gh:c4deszes/samd21-hal#feature/atsamc21-support")
CPMAddPackage("gh:c4deszes/bike-line-protocol@0.1.0")
CPMAddPackage("gh:c4deszes/bike-flash-tool@0.1.0")
CPMAddPackage("gh:c4deszes/bike-uds-tool#master")

enable_testing()
include(CTest)
set(CTEST_OUTPUT_ON_FAILURE ON)

include(CTest)
include(GoogleTest)

line_codegen(
    TARGET protocol-stack-api
    NETWORK ../customer/network.json
    NODE RearLight
    ADAPTER
)

uds_codegen(
    TARGET uds-lib
    PROFILE tools/line/uds_profile.json
)

samx21_hal(samc21-hal ATSAMC21E18A)

# add_library(
#     bma456mm INTERFACE
# )
# target_sources(bma456mm INTERFACE
#     lib/bma456/bma4.c
#     lib/bma456/bma456mm.c
# )
# target_include_directories(bma456mm INTERFACE lib/bma456)

set(APP_GENCODE_DIR ${CMAKE_CURRENT_BINARY_DIR}/gencode)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/app/metainfo.h.in
               ${APP_GENCODE_DIR}/metainfo.h)

add_executable(
    application
    EXCLUDE_FROM_ALL

    src/app/brake.c
    src/app/brightness.c
    src/app/comm.c
    src/app/init.c
    src/app/main.c
    src/app/strobe.c
    src/app/sys_state.c
    src/app/tasks.c

    src/bsp/board.c
    src/bsp/led_driver_tld509x.c
    src/bsp/usart.c
)
target_include_directories(application PRIVATE include ${APP_GENCODE_DIR})
target_link_libraries(application PRIVATE   cmsis
                                            samc21-hal
                                            flash-line-api flash-line-sources
                                            protocol-stack-api
                                            uds-lib
                                            # bootloader-api
                                            # bma456mm
                                            )

set_target_properties(application PROPERTIES SUFFIX ".elf")
# target_link_directories(application PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ld)
# target_link_options(application PRIVATE -T${CMAKE_CURRENT_SOURCE_DIR}/ld/application.ld)
target_link_options(application PRIVATE -T$<TARGET_PROPERTY:samc21-hal,DEFAULT_LINKERSCRIPT>)
target_link_options(application PRIVATE -Wl,-Map=application.map -Wl,--gc-sections)

hexify(TARGET application HEX ${CMAKE_CURRENT_BINARY_DIR}/application_noheader.hex)
dump_symbols(application ${CMAKE_CURRENT_BINARY_DIR}/application.lss)
dump_size(application)

# bootloader_header(
#     TARGET factory_header
#     CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/tools/bootloader/boot_config.json
#     OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/tools/openocd/factory_header.hex
# )

# bootloader_header(
#     TARGET app_header
#     CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/tools/bootloader/boot_config.json
#     APPLICATION ${CMAKE_CURRENT_BINARY_DIR}/application_noheader.hex
#     OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/app_header.hex
# )

# add_custom_target(application-merge
#     DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/app_header.hex ${CMAKE_CURRENT_BINARY_DIR}/application_noheader.hex
#     BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/application.hex
#     COMMAND srec_cat
#             ${CMAKE_CURRENT_BINARY_DIR}/application_noheader.hex -intel
#             ${CMAKE_CURRENT_BINARY_DIR}/app_header.hex -intel
#             -o ${CMAKE_CURRENT_BINARY_DIR}/application.hex -intel
# )

add_custom_target(application-full
    DEPENDS application-hex application-lss application-memory
)
