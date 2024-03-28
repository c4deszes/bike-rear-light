# This function converts an .elf file into IntelHEX file
# Usage:
#   add_executable(MyApp main.c)
#   hexify(MyApp MyApp.hex)
# Prerequisites:
#   CMAKE_OBJCOPY tool configured in the toolchain
#   TARGET is an executable that can be converted into a hex file
function(hexify TARGET HEX)
    add_custom_command(
        DEPENDS ${TARGET}
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${TARGET}> ${HEX}
        OUTPUT ${HEX}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "\tCreating hex output: ${HEX}")
    add_custom_target(${TARGET}-hex DEPENDS ${HEX})
endfunction()

# function(pymcuprog HEX DEVICE)
#     add_custom_command(
#         DEPENDS ${HEX}
#         COMMAND "pymcuprog" -C -d ${DEVICE} write -f ${HEX}
#         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#         OUTPUT ${}
#     )
# endfunction()

# This function converts an .elf file into a binary file
# Usage:
#   add_executable(MyApp main.c)
#   binarize(MyApp MyApp.bin)
# Prerequisites:
#   CMAKE_OBJCOPY tool configured in the toolchain
#   TARGET is an executable that can be converted into a binary file
function(binarize TARGET BIN)
    add_custom_command(
        DEPENDS ${TARGET}
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${TARGET}> ${BIN}
        OUTPUT ${BIN}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "\tCreating binary output: ${BIN}")
    add_custom_target(${TARGET}-bin DEPENDS ${BIN})
endfunction()

# This function converts an .elf file into an .lss file by dumping symbol
# information.
# Usage:
#   add_executable(MyApp main.c)
#   dump_symbols(MyApp MyApp.lss)
# Prerequisites:
#   CMAKE_OBJDUMP tool configured in the toolchain
#   TARGET is an executable
function(dump_symbols TARGET LSS)
    add_custom_command(
        DEPENDS ${TARGET}
        COMMAND ${CMAKE_OBJDUMP} -h -S $<TARGET_FILE:${TARGET}> > ${LSS}
        OUTPUT ${LSS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "\tCreating extended listing: ${LSS}")
    add_custom_target(${TARGET}-lss DEPENDS ${LSS})
endfunction()

# This function prints out the size of an .elf file
# Usage:
#   add_executable(MyApp main.c)
#   dump_size(MyApp)
# Prerequisites:
#   CMAKE_OBJSIZE tool configured in the toolchain
#   TARGET is an executable
function(dump_size TARGET)
    add_custom_command(
        DEPENDS ${TARGET}
        COMMAND ${CMAKE_OBJSIZE} $<TARGET_FILE:${TARGET}>
        OUTPUT ${TARGET}-memory
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "\tCreating memory usage report ${TARGET}")
endfunction()

# This function writes the given hex files 
# function(write_program TARGET HEX)
#     add_custom_command(
#         DEPENDS ${HEX}
#         COMMAND "pymcuprog write -C -p COM3 -f ${HEX}"
#         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#         COMMENT "\tCreating memory usage report ${TARGET}")
# endfunction()