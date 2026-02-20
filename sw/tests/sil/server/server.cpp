#include "rear_light.capnp.h"
#include <kj/debug.h>
#include <capnp/ez-rpc.h>
#include <capnp/message.h>
#include <iostream>
#include <queue>

extern "C" {

#include "common/scheduler.h"

#include "sim/dsu_internal.h"
#include "lightsim.h"

#include "app/init.h"
#include "line_protocol.h"

}


uint32_t DSU_SerialNumber = 0x12345678;

/******************************************************************************/

class RearLightImpl final: public RearLight::Server {

public:

    kj::Promise<void> initialize(InitializeContext context) override {
        // Simulate initialization.
        auto params = context.getParams();
        auto results = context.getResults();

        APP_Init();

        return kj::READY_NOW;
    }

    kj::Promise<void> onTick(OnTickContext context) override {
        auto params = context.getParams();

        for (int i = 0; i < params.getMillis(); i++) {
            SCH_Trigger();
            SCH_Step();
        }
        return kj::READY_NOW;
    }

    kj::Promise<void> destroy(DestroyContext context) override {
        // Simulate cleanup.
        return kj::READY_NOW;
    }

    kj::Promise<void> setTailLightState(SetTailLightStateContext context) override {
        // Simulate setting the sensor state.
        auto params = context.getParams();
        auto state = params.getState();

        // Simulate setting the sensor state based on the input.
        if (state == RearLight::LightFeatureState::OK) {
            LIGHTSIM_SetTailLightState(lightcontrol_feature_state_ok);
        } else if (state == RearLight::LightFeatureState::PARTIAL_ERROR) {
            LIGHTSIM_SetTailLightState(lightcontrol_feature_state_partial_error);
        } else if (state == RearLight::LightFeatureState::ERROR) {
            LIGHTSIM_SetTailLightState(lightcontrol_feature_state_error);
        }

        return kj::READY_NOW;
    }

    kj::Promise<void> getTailLightBrightness(GetTailLightBrightnessContext context) override {
        // Simulate getting the sensor state.
        auto results = context.getResults();

        // Simulate getting the sensor state and returning it.
        uint16_t brightness = LIGHTSIM_GetTailLightBrightness();
        results.setBrightness(brightness);
        return kj::READY_NOW;
    }

    kj::Promise<void> onRequest(OnRequestContext context) override {
        auto params = context.getParams();
        auto results = context.getResults();

        auto request = params.getRequest();

        bool respondsTo = LINE_Transport_RespondsTo(0, request);
        if (respondsTo) {
            uint8_t size;
            uint8_t buffer[256];
            LINE_Transport_PrepareResponse(0, request, &size, buffer);
            capnp::Data::Reader payload(buffer, size);
            results.setPayload(payload);
        } else {
            capnp::Data::Reader payload;
            results.setPayload(payload);
        }

        return kj::READY_NOW;
    }

    kj::Promise<void> onRequestComplete(OnRequestCompleteContext context) override {
        auto params = context.getParams();
        auto request = params.getRequest();
        auto payload = params.getPayload().asBytes();

        uint8_t size = payload.size();
        uint8_t buffer[256];
        for (size_t i = 0; i < payload.size(); i++) {
            buffer[i] = payload[i];
        }

        std::cout << "Received request with size " << (int)size << std::endl;

        LINE_Transport_OnData(0, false, request, size, buffer);

        std::cout << "Handled request" << std::endl;

        return kj::READY_NOW;
    }
};

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " ADDRESS[:PORT]\n"
        "Runs the server bound to the given address/port.\n"
        "ADDRESS may be '*' to bind to all local addresses.\n"
        ":PORT may be omitted to choose a port automatically." << std::endl;
    return 1;
  }

  // Set up a server.
  capnp::EzRpcServer server(kj::heap<RearLightImpl>(), argv[1]);

  // Write the port number to stdout, in case it was chosen automatically.
  auto& waitScope = server.getWaitScope();
  unsigned int port = server.getPort().wait(waitScope);
  if (port == 0) {
    // The address format "unix:/path/to/socket" opens a unix domain socket,
    // in which case the port will be zero.
    std::cout << "Listening on Unix socket..." << std::endl;
  } else {
    std::cout << "Listening on port " << port << "..." << std::endl;
  }

  // Run forever, accepting connections and handling requests.
  kj::NEVER_DONE.wait(waitScope);
}
