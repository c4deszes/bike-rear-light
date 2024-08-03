Brake component design
======================

The brake component is responsible for:

* Initializing the accelerometer
* Polling the accelerometer
* Determining braking condition based on the accelerometer data
* Determining braking condition based on signals coming from the RotorSensor peripheral

Initialization
--------------

The accelerometer is configured:

* Data rate: 100Hz
* Range: 2G
* Averaging: 4 samples (CIC)

If the sensor initialization fails for any reason the sensor is put into permanent failure state,
initialization will only be retried on a power-on-reset.

Operation
---------

During operation if the sensor initialized correctly then the acceleration data is polled and is
used by an algorithm to detect braking.
