add_library(bsp-config INTERFACE)
target_include_directories(bsp-config INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(bsp-config PROPERTIES BSP_IMU_DRIVER NONE)
set_target_properties(bsp-config PROPERTIES BSP_LED_DRIVER TLD509x)

gen_user_config(userconf-tld509x-default tld509x-default.c)
add_dependencies(bsp-config userconf-tld509x-default)
get_target_property(BSP_DEFAULT_USERCONFIG userconf-tld509x-default HEX)
set_target_properties(bsp-config PROPERTIES BSP_DEFAULT_USERCONFIG ${BSP_DEFAULT_USERCONFIG})

set_target_properties(bsp-config PROPERTIES BSP_BOOT_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/boot_config.json)
