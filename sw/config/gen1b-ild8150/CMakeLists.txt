add_library(bsp-config INTERFACE)
target_include_directories(bsp-config INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(bsp-config PROPERTIES BSP_IMU_DRIVER NONE)
set_target_properties(bsp-config PROPERTIES BSP_LED_DRIVER ILD8150)
set_target_properties(bsp-config PROPERTIES BSP_RADAR_MODULE NONE)

gen_user_config(userconf-ild8150-default ild8150-default.c)
add_dependencies(bsp-config userconf-ild8150-default)
get_target_property(BSP_DEFAULT_USERCONFIG userconf-ild8150-default HEX)
set_target_properties(bsp-config PROPERTIES BSP_DEFAULT_USERCONFIG ${BSP_DEFAULT_USERCONFIG})

set_target_properties(bsp-config PROPERTIES BSP_BOOT_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/boot_config.json)
