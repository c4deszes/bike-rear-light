add_library(bsp-config INTERFACE)
target_include_directories(bsp-config INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(bsp-config PROPERTIES BSP_IMU_DRIVER BMA456)
set_target_properties(bsp-config PROPERTIES BSP_LED_DRIVER TLD2xxx)

gen_user_config(userconf-tld2xxx-default tld2xxx-default.c)
add_dependencies(bsp-config userconf-tld2xxx-default)
get_target_property(BSP_DEFAULT_USERCONFIG userconf-tld2xxx-default HEX)
set_target_properties(bsp-config PROPERTIES BSP_DEFAULT_USERCONFIG ${BSP_DEFAULT_USERCONFIG})

set_target_properties(bsp-config PROPERTIES BSP_BOOT_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/boot_config.json)
