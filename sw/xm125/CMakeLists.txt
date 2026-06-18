
set(STM32L4XX_HAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/STM32CubeL4)

add_library(stm32l4xx_hal INTERFACE)
target_sources(stm32l4xx_hal INTERFACE
    ${STM32L4XX_HAL_DIR}/Drivers/CMSIS/Device/ST/STM32L4xx/Source/Templates/gcc/startup_stm32l431xx.s
    ${STM32L4XX_HAL_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c
    ${STM32L4XX_HAL_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c
    ${STM32L4XX_HAL_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.c
    ${STM32L4XX_HAL_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c
    ${STM32L4XX_HAL_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c.c
    ${STM32L4XX_HAL_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.c
    ${STM32L4XX_HAL_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c
    ${STM32L4XX_HAL_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c
    ${STM32L4XX_HAL_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc.c
    ${STM32L4XX_HAL_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.c
    ${STM32L4XX_HAL_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c
    ${STM32L4XX_HAL_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc.c
    ${STM32L4XX_HAL_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc_ex.c
    ${STM32L4XX_HAL_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi.c
    ${STM32L4XX_HAL_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart.c
    ${STM32L4XX_HAL_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart_ex.c
)
target_include_directories(stm32l4xx_hal INTERFACE
    ${STM32L4XX_HAL_DIR}/Drivers/CMSIS/Include
    ${STM32L4XX_HAL_DIR}/Drivers/STM32L4xx_HAL_Driver/Inc
    ${STM32L4XX_HAL_DIR}/Drivers/CMSIS/Device/ST/STM32L4xx/Include
)
# TODO: add flags -Wno-pedantic and -Wno-embedded-directive on these files
target_compile_options(stm32l4xx_hal INTERFACE
    -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
)
target_link_options(stm32l4xx_hal INTERFACE
    -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
)
target_compile_definitions(stm32l4xx_hal INTERFACE
    -DSTM32L431xx -DUSE_HAL_DRIVER
    -DTARGET_ARCH_cm4 -DFLOAT_ABI_HARD
)

add_library(xm125-api INTERFACE)
target_include_directories(xm125-api INTERFACE api)

add_executable(xm125
    EXCLUDE_FROM_ALL
    src/main.c
    src/stm32l4xx_hal_msp.c
    src/stm32l4xx_it.c
    src/syscalls.c
    src/sysmem.c
    src/system_stm32l4xx.c

    src/algorithms/acc_algorithm.c
    src/protocol/acc_reg_protocol.c
    src/protocol/i2c_application_system_stm32.c

    apps/car_detector.c
    apps/car_detector_i2c.c

    src/integration/acc_hal_integration_stm32cube_xm.c
    src/integration/acc_integration_cortex.c
    src/integration/acc_integration_log.c
    src/integration/acc_integration_stm32.c
    src/integration/acc_wrap_printf.c
    src/integration/printf.c
)

target_compile_definitions(xm125 PRIVATE
    -DPRINTF_DISABLE_SUPPORT_FLOAT -DPRINTF_DISABLE_SUPPORT_EXPONENTIAL # TODO: only for printf file
)
target_include_directories(xm125 PRIVATE include api apps)
target_compile_options(xm125 PRIVATE
    -pedantic
    -Wall #-Werror
    -Wextra -Wdouble-promotion
    -Wstrict-prototypes -Wcast-qual #-Wmissing-prototypes
    -Winit-self -Wpointer-arith -Wshadow -MP
    -fno-math-errno -ffunction-sections -fdata-sections
)

set_target_properties(xm125 PROPERTIES SUFFIX ".elf")
set(XM125_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/config/STM32L431CBYx_FLASH.ld)
set_target_properties(xm125 PROPERTIES LINK_DEPENDS ${XM125_LINKER_SCRIPT})
target_link_options(xm125 PRIVATE
    -fno-lto
    -Wl,--gc-sections
    -T${XM125_LINKER_SCRIPT}
)
target_link_directories(xm125 PRIVATE lib)
target_link_libraries(xm125 PRIVATE
    stm32l4xx_hal
    acconeer_a121
)

# Post processing
hexify(TARGET xm125 HEX ${CMAKE_CURRENT_BINARY_DIR}/xm125.hex)
dump_symbols(xm125 ${CMAKE_CURRENT_BINARY_DIR}/xm125.lss)
dump_size(xm125)

add_custom_target(xm125-full DEPENDS xm125-hex xm125-lss xm125-memory)
