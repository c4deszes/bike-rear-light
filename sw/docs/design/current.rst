Current component design
========================

The current component is responsible for:

* Estimating the current used by the device
* Calculating the derating factor based on voltage and temperature

Initialization
--------------

* Current estimate is set to zero
* Derating factor is set to it's maximum

Operation
---------

* Current estimate is calculated based on the duty cycle and the battery voltage
* Derating factor is calculated based on the battery voltage and temperature
* If either the voltage or temperature is out of the valid range, the derating factor is set to maximum
