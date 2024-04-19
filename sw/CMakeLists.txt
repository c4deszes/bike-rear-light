cmake_minimum_required(VERSION 3.20)

project(
    rear-light
    DESCRIPTION "Rear Light"
    LANGUAGES C CXX ASM)
set(PROJECT_VERSION "0.1.0")

find_package(Python REQUIRED)

include(tools/cmake/CPM.cmake)

CPMAddPackage("gh:c4deszes/samd21-hal#feature/tcc-improvements")
CPMAddPackage("gh:c4deszes/bike-line-protocol#feature/python-tool-improvements")
CPMAddPackage("gh:c4deszes/bike-flash-tool#master")
CPMAddPackage("gh:c4deszes/samd21-line-bootloader#master")

# Test configuration
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

add_library(
    bma456mm INTERFACE
)
target_sources(bma456mm INTERFACE
    lib/bma456/bma4.c
    lib/bma456/bma456mm.c
)
target_include_directories(bma456mm INTERFACE lib/bma456)

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
    src/bsp/light_control.c
    src/bsp/usart.c
)
target_include_directories(application PRIVATE include)
target_link_libraries(application PRIVATE   cmsis
                                            atsamd21e18a-dfp
                                            samd21-hal-api
                                            samd21-hal-sources
                                            samd21-hal-options
                                            flash-line-api flash-line-sources
                                            protocol-stack-api
                                            bootloader-api
                                            bma456mm)

set_target_properties(application PROPERTIES SUFFIX ".elf")
target_link_directories(application PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ld)
target_link_options(application PRIVATE -T${CMAKE_CURRENT_SOURCE_DIR}/ld/application.ld)
target_link_options(application PRIVATE -Wl,-Map=application.map -Wl,--gc-sections)

hexify(TARGET application HEX ${CMAKE_CURRENT_BINARY_DIR}/application_noheader.hex)
dump_symbols(application ${CMAKE_CURRENT_BINARY_DIR}/application.lss)
dump_size(application)

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

add_custom_target(application-full
    DEPENDS application-hex application-lss application-memory application-merge factory_header
)
