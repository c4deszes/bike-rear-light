# Compiler and linker flags that enable Link time optimizations
add_library(gcc-lto INTERFACE)
target_compile_options(gcc-lto INTERFACE
    "-flto" "-ffunction-sections" "-fdata-sections" "-ffat-lto-objects")
target_link_options(gcc-lto INTERFACE "-Wl,-flto")

function(enable_lto TARGET)
    target_link_libraries(${TARGET} PRIVATE gcc-lto)
endfunction()
