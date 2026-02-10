@0xabcdefabcdefabcdef;

interface LineTransport {
    onRequest @0 (request: UInt16) -> (payload: Data);
    onRequestComplete @1 (request: UInt16, payload: Data) -> ();
}

interface RearLight extends(LineTransport) {
    initialize @0 () -> ();
    onTick @1 (millis: Int32 = 1) -> ();
    destroy @2 () -> ();

    # Microcontroller
    hasResetRequest @3 () -> (hasRequest: Bool);
    hasSleepRequest @4 () -> (hasRequest: Bool);

    # Light control emulation
    enum LightFeatureState {
        ok @0;
        partialError @1;
        error @2;
    }
    setTailLightState @5 (state: LightFeatureState);
    getTailLightBrightness @6 () -> (brightness: UInt16);
}
