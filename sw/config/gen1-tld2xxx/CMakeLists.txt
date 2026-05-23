add_library(bsp-config INTERFACE)
target_include_directories(bsp-config INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(bsp-config PROPERTIES BSP_IMU_DRIVER BMA456)
set_target_properties(bsp-config PROPERTIES BSP_LED_DRIVER TLD2xxx)

# TODO: Set default user configuration

set_target_properties(bsp-config PROPERTIES BSP_BOOT_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/boot_config.json)
