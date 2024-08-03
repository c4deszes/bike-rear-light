System state component design
=============================

System state manages the highest level states the rear light might have.

Initialization
--------------

The light starts in ``Init`` state, in this mode the lights are at maximum brightness.
The controller is not actually active in this case, rather hardware measures enable the
light by default.

Operation
---------

After initialization the light goes either into normal or safety mode.

* In normal mode the light uses the communicated target brightness, strobe mode and other settings.
* In safety mode the light uses the internally programmed fixed brightness and strobe mode.

The device transitions into safety mode if communication is lost for 4 seconds, with one exception
that is if the light has been instructed to use it's emergency brightness. This is a mode where the
system tries to save power and communication might be lost due to the battery draining.

The device may be put into idle and shutdown diagnostic modes.

* In idle mode the device instantly transitions to safety mode
* In shutdown mode the device goes into deep sleep, turning off all outputs
