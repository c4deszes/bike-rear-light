cmake_minimum_required(VERSION 3.20)

project(
    rear-light
    DESCRIPTION "Rear Light"
    LANGUAGES C CXX ASM
    VERSION "1.0.0")

################################################################################
# Options
################################################################################
option(REAR_LIGHT_BUILD_UNITTESTS "Enables unit test cases" OFF)
option(REAR_LIGHT_BUILD_SILTESTS "Enables integration test cases" OFF)
option(REAR_LIGHT_BUILD_FUNCTESTS "Enables functional test cases" OFF)
option(REAR_LIGHT_BUILD_DOCS "Enables documentation generation" OFF)

find_package(Python REQUIRED)
include(tools/cmake/CPM.cmake)

################################################################################
# Dependencies
################################################################################
set(BIKE_LINE_PROTOCOL_VERSION feature/codegen-exec)
set(BIKE_UDS_TOOL_VERSION feature/service-call-new)

CPMAddPackage("gh:c4deszes/samd21-hal#feature/sim-support")
CPMAddPackage("gh:c4deszes/bike-line-protocol#${BIKE_LINE_PROTOCOL_VERSION}")
CPMAddPackage("gh:c4deszes/samd21-line-bootloader@0.4.0")
CPMAddPackage("gh:c4deszes/bike-flash-tool@0.2.0")
CPMAddPackage("gh:c4deszes/bike-uds-tool#${BIKE_UDS_TOOL_VERSION}")

################################################################################
# Tools setup
################################################################################
add_custom_target(
    PythonSetup
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}

    COMMAND ${CMAKE_COMMAND} -E make_directory tools/python
    #COMMAND git clone -b ${BIKE_LINE_PROTOCOL_VERSION} https://github.com/c4deszes/bike-line-protocol tools/python/bike-line-protocol
    COMMAND pip install -e tools/python/bike-line-protocol/python-lib

    #COMMAND git clone -b ${BIKE_UDS_TOOL_VERSION} https://github.com/c4deszes/bike-uds-tool tools/python/bike-uds-tool
    COMMAND pip install -e tools/python/bike-uds-tool/python-lib
)

################################################################################
# Testing
################################################################################
enable_testing()
include(CTest)
set(CTEST_OUTPUT_ON_FAILURE ON)

include(CTest)
include(GoogleTest)
include(tools/cmake/Pytest.cmake)

################################################################################
# Libraries
################################################################################
samx21_hal(samd21e18a ATSAMD21E18A)

add_library(bma456mm-api INTERFACE)
target_include_directories(bma456mm-api INTERFACE lib/bma456)

add_library(bma456mm INTERFACE)
target_sources(bma456mm INTERFACE
    lib/bma456/bma4.c
    lib/bma456/bma456mm.c
)
target_link_libraries(bma456mm INTERFACE bma456mm-api)

################################################################################
# Code generation and application
################################################################################
line_codegen(
    TARGET protocol-stack-api
    CONFIG tools/line/line_codegen.json
    ADAPTER
)

uds_codegen(
    TARGET uds-lib
    CONFIG tools/line/uds_codegen.json
)

set(APP_GENCODE_DIR ${CMAKE_CURRENT_BINARY_DIR}/gencode)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/app/metainfo.h.in
               ${APP_GENCODE_DIR}/metainfo.h)

################################################################################
# Board configurations
################################################################################
add_subdirectory(config/gen1b-tld509x)

add_library(bsp INTERFACE)
target_link_libraries(bsp INTERFACE samd21e18a bsp-config)
target_sources(bsp INTERFACE
    src/bsp/board.c

    src/bsp/usart.c
    src/bsp/line_usart.c
    src/bsp/tt_adc.c
)

# Sensor driver selection
get_target_property(BSP_SENSOR_DRIVER bsp-config BSP_SENSOR_DRIVER)
if (BSP_SENSOR_DRIVER STREQUAL "BMA456")
    target_sources(bsp INTERFACE src/bsp/accel_bma456.c)
    target_link_libraries(bsp INTERFACE bma456mm)
elseif(BSP_SENSOR_DRIVER STREQUAL "NONE")
    target_sources(bsp INTERFACE src/bsp/accel_none.c)
else()
    message(FATAL_ERROR "Unsupported BSP_SENSOR_DRIVER: ${BSP_SENSOR_DRIVER}")
endif()

# LED driver selection
get_target_property(BSP_LED_DRIVER bsp-config BSP_LED_DRIVER)
if (BSP_LED_DRIVER STREQUAL "TLD509x")
    target_sources(bsp INTERFACE src/bsp/led_driver_tld509x.c)
else()
    message(FATAL_ERROR "Unsupported BSP_LED_DRIVER: ${BSP_LED_DRIVER}")
endif()

################################################################################
# Application
################################################################################
add_library(application-api INTERFACE)
target_sources(application-api INTERFACE
    src/app/brake.c
    src/app/brightness.c
    src/app/config.c
    src/app/comm.c
    src/app/current.c
    src/app/diag.c
    src/app/init.c
    src/app/strobe.c
    src/app/sys_state.c
    src/app/tasks.c
    src/app/temp.c
    src/app/volt.c
)
target_include_directories(application-api INTERFACE include ${APP_GENCODE_DIR})
target_link_libraries(application-api INTERFACE
    flash-line-api flash-line-sources
    uds-lib
    bootloader-api)

add_executable(
    application
    EXCLUDE_FROM_ALL

    src/app/main.c
)

target_link_libraries(application PRIVATE   bsp
                                            protocol-stack-api
                                            application-api)

set_target_properties(application PROPERTIES SUFFIX ".elf")
target_link_options(application PRIVATE -T${CMAKE_CURRENT_SOURCE_DIR}/ld/application.ld)
target_link_options(application PRIVATE -Wl,-Map=application.map -Wl,--gc-sections)

# Post processing
hexify(TARGET application HEX ${CMAKE_CURRENT_BINARY_DIR}/application_noheader.hex)
dump_symbols(application ${CMAKE_CURRENT_BINARY_DIR}/application.lss)
dump_size(application)

####################################################################################################
# Bootloader
####################################################################################################
bootloader_header(
    TARGET factory_header
    CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/tools/bootloader/boot_config.json
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/tools/openocd/factory_header.hex
)

bootloader_header(
    TARGET app_header
    CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/tools/bootloader/boot_config.json
    APPLICATION ${CMAKE_CURRENT_BINARY_DIR}/application_noheader.hex
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/app_header.hex
)

add_custom_target(application-merge
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/app_header.hex ${CMAKE_CURRENT_BINARY_DIR}/application_noheader.hex
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/application.hex
    COMMAND srec_cat
            ${CMAKE_CURRENT_BINARY_DIR}/application_noheader.hex -intel
            ${CMAKE_CURRENT_BINARY_DIR}/app_header.hex -intel
            -o ${CMAKE_CURRENT_BINARY_DIR}/application.hex -intel
)

add_custom_target(application-debug
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/app_header.hex ${CMAKE_CURRENT_BINARY_DIR}/application_noheader.hex
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/application-debug.hex
    COMMAND srec_cat
            ${CMAKE_CURRENT_BINARY_DIR}/application_noheader.hex -intel
            ${CMAKE_CURRENT_BINARY_DIR}/app_header.hex -intel
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/bootloader/bootloader_with_crc.hex -intel
            -o ${CMAKE_CURRENT_BINARY_DIR}/application-debug.hex -intel
)

add_custom_target(application-full
    DEPENDS application-hex application-lss application-memory
            application-merge application-debug factory_header
)

################################################################################
# Tests
################################################################################
if (REAR_LIGHT_BUILD_UNITTESTS)
    add_subdirectory(tests/unit)
endif()

if (REAR_LIGHT_BUILD_SILTESTS)
    add_subdirectory(tests/sil)
endif()

if (REAR_LIGHT_BUILD_FUNCTESTS)
    add_subdirectory(tests/pytest)
endif()

##############################################################################
# Documentation
##############################################################################
if (REAR_LIGHT_BUILD_DOCS)
    add_subdirectory(docs)
endif()
