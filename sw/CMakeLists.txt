cmake_minimum_required(VERSION 3.20)

project(
    rear-light
    DESCRIPTION "Rear Light"
    LANGUAGES C CXX ASM
    VERSION "2.0.0")

################################################################################
# Git info
################################################################################
set(PROJECT_GIT_SHA "unknown")
set(PROJECT_GIT_DIRTY "1")
execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE PROJECT_GIT_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND git diff --quiet
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    RESULT_VARIABLE GIT_DIFF_RESULT
)
if(GIT_DIFF_RESULT EQUAL 0)
    set(PROJECT_GIT_DIRTY "0")
endif()

################################################################################
# Options
################################################################################
option(REAR_LIGHT_BUILD_UNITTESTS "Enables unit test cases" OFF)
option(REAR_LIGHT_BUILD_SILTESTS "Enables integration test cases" OFF)
option(REAR_LIGHT_BUILD_FUNCTESTS "Enables functional test cases" OFF)
option(REAR_LIGHT_BUILD_DOCS "Enables documentation generation" OFF)

if (NOT DEFINED REAR_LIGHT_BOARD_VARIANT)
    message(FATAL_ERROR "REAR_LIGHT_BOARD_VARIANT not set. Please set it to a valid board variant (e.g. gen2-al8891).")
endif()

find_package(Python REQUIRED)
include(tools/cmake/CPM.cmake)

################################################################################
# Dependencies
################################################################################
set(BIKE_LINE_PROTOCOL_VERSION v0.4.0)
set(BIKE_UDS_TOOL_VERSION v0.3.0)

CPMAddPackage("gh:c4deszes/samd21-hal@0.5.0")
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
# User configurations
################################################################################
add_subdirectory(config/data)

################################################################################
# Board configurations
################################################################################
add_subdirectory(config/${REAR_LIGHT_BOARD_VARIANT})

add_library(bsp INTERFACE)
target_link_libraries(bsp INTERFACE samd21e18a bsp-config)
target_sources(bsp INTERFACE
    src/bsp/board.c

    src/bsp/usart.c
    src/bsp/line_usart.c
    src/bsp/tt_adc.c
)

# Sensor driver selection
get_target_property(BSP_IMU_DRIVER bsp-config BSP_IMU_DRIVER)
if (BSP_IMU_DRIVER STREQUAL "BMA456")
    target_sources(bsp INTERFACE src/bsp/accel_bma456.c)
    target_link_libraries(bsp INTERFACE bma456mm)
elseif(BSP_IMU_DRIVER STREQUAL "NONE")
    target_sources(bsp INTERFACE src/bsp/accel_none.c)
else()
    message(FATAL_ERROR "Unsupported BSP_IMU_DRIVER: ${BSP_IMU_DRIVER}")
endif()

# LED driver selection
get_target_property(BSP_LED_DRIVER bsp-config BSP_LED_DRIVER)
if (BSP_LED_DRIVER STREQUAL "TLD509x")
    target_sources(bsp INTERFACE src/bsp/led_driver_tld509x.c)
elseif (BSP_LED_DRIVER STREQUAL "ILD8150")
    target_sources(bsp INTERFACE src/bsp/led_driver_ild8150.c)
elseif (BSP_LED_DRIVER STREQUAL "TLD2xxx")
    target_sources(bsp INTERFACE src/bsp/led_driver_tld2xxx.c)
elseif (BSP_LED_DRIVER STREQUAL "AL8891")
    target_sources(bsp INTERFACE src/bsp/led_driver_al8891.c)
else()
    message(FATAL_ERROR "Unsupported BSP_LED_DRIVER: ${BSP_LED_DRIVER}")
endif()

# Default user configuration
get_target_property(BSP_DEFAULT_USERCONFIG bsp-config BSP_DEFAULT_USERCONFIG)
if (NOT BSP_DEFAULT_USERCONFIG)
    message(FATAL_ERROR "BSP_DEFAULT_USERCONFIG not set in bsp-config")
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
    src/app/router.c
    src/app/strobe.c
    src/app/sys_state.c
    src/app/tasks.c
    src/app/temp.c
    src/app/volt.c
    src/app/calib.c
    src/app/wake.c
)
target_include_directories(application-api INTERFACE include ${APP_GENCODE_DIR})
target_link_libraries(application-api INTERFACE
    flash-line-api flash-line-sources
    uds-lib
    bootloader-api)

add_executable(application EXCLUDE_FROM_ALL
    src/app/main.c
)

target_link_libraries(application PRIVATE   bsp
                                            protocol-stack-api
                                            application-api)

set_target_properties(application PROPERTIES SUFFIX ".elf")
target_link_options(application PRIVATE -T${CMAKE_CURRENT_SOURCE_DIR}/ld/application.ld)
target_link_options(application PRIVATE -Wl,-Map=application.map -Wl,--gc-sections)

# Calibration
add_subdirectory(config/calib)
get_target_property(CALIB_DEFAULT_HEX calib-default HEX)

# Post processing
hexify(TARGET application HEX ${CMAKE_CURRENT_BINARY_DIR}/application_noheader.hex)
dump_symbols(application ${CMAKE_CURRENT_BINARY_DIR}/application.lss)
dump_size(application)

####################################################################################################
# Bootloader
####################################################################################################
get_target_property(BSP_BOOT_CONFIG bsp-config BSP_BOOT_CONFIG)

bootloader_header(
    TARGET factory_header
    CONFIG ${BSP_BOOT_CONFIG}
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/tools/openocd/factory_header.hex
)

bootloader_header(
    TARGET app_header
    CONFIG ${BSP_BOOT_CONFIG}
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
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/app_header.hex
            ${CMAKE_CURRENT_BINARY_DIR}/application_noheader.hex
            ${BSP_DEFAULT_USERCONFIG}
            calib-default
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/application-debug.hex
    COMMAND srec_cat
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/bootloader/bootloader_with_crc.hex -intel
            ${CMAKE_CURRENT_BINARY_DIR}/application_noheader.hex -intel
            ${BSP_DEFAULT_USERCONFIG} -intel
            ${CALIB_DEFAULT_HEX} -intel
            ${CMAKE_CURRENT_BINARY_DIR}/app_header.hex -intel
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
