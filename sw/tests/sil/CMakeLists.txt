####################################################################################################
# Dependencies
####################################################################################################
CPMAddPackage("gh:capnproto/capnproto@1.1.0")

####################################################################################################
# Code generation
####################################################################################################
capnp_generate_cpp(rear-light-rpc-sources rear-light-rpc-headers server/rear_light.capnp)

####################################################################################################
# Software in loop (SIL) test application
####################################################################################################
line_codegen(
    TARGET protocol-sil-lib
    CONFIG ../../tools/line/line_codegen.json
    ADAPTER
    NO_TRANSPORT
)

add_executable(rear-light-server EXCLUDE_FROM_ALL
    ${rear-light-rpc-sources}
    server/server.cpp

    stubs/boardsim.c
    stubs/lightsim.c
    stubs/accel_bma456sim.c
)
target_include_directories(rear-light-server PRIVATE
    $<PATH:REMOVE_FILENAME,${rear-light-rpc-headers}>     # TODO: why is this needed?
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs)

target_link_libraries(rear-light-server PRIVATE
    samx21-api
    samx21-sim
    bma456mm-api
    protocol-sil-lib
    application-api
)

target_link_libraries(rear-light-server PRIVATE CapnProto::capnp CapnProto::capnp-rpc)
target_compile_options(rear-light-server PRIVATE -Wno-deprecated-declarations -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -Wno-missing-field-initializers)
