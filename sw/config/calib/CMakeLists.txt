
function(gen_calib_data target_name source_file)
    add_executable(${target_name} EXCLUDE_FROM_ALL)
    target_sources(${target_name} PRIVATE ${source_file})
    target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
    target_link_options(${target_name} PRIVATE -nostdlib -T${CMAKE_CURRENT_SOURCE_DIR}/calib.ld)
    add_custom_command(
        TARGET ${target_name} POST_BUILD
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.hex
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${target_name}> ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.hex
        COMMAND srec_cat ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.hex -Intel -crop 0x3F200 0x3F2FC
                -CRC32_Little_endian 0x3F2FC --CCITT -o ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.hex -Intel
        COMMENT "Generating user configuration: ${target_name}"
    )
    set_target_properties(${target_name} PROPERTIES HEX ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.hex)
endfunction()

gen_calib_data(calib-default calib-default.c)
