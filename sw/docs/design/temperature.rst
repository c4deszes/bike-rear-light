Temperature component design
============================

Temperature component is responsible for monitoring the temperature of the device
and updating the temperature status accordingly.

Initialization
--------------

The components sets up the microcontroller pin to read an external NTC.

Initially the temperature status is set to okay and the temperature is set to 25°C.

Operation
---------

* Temperature is measured
* Temperature status is updated, based on the temperature sensor reading
