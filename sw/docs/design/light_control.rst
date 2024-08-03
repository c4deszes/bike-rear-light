Light control component design
==============================

Light control is responsible for setting the PWM outputs according to the target duty cycle,
it's also responsible for detecting errors and diagnosing the output stages.

Initialization
--------------

The component sets up the TCC1 peripheral to output a 1kHz PWM signal with variable duty cycle.
The outputs are inverted because the drivers use an active low signal.

Initially PWM output is set to the maximum for the taillight and minimum for the brakelight,
this conforms to the hardware default state.

Operation
---------

Component is updated every 10ms, in this call:

* PWM duty cycle is reevaluated
* Output stage errors are detected
* Diagnosis requests are handled

Diagnosis
---------

Tail light is evaluated segment by segment, initially all segments are off then the driver cycles
through them identifying the segments with short or open circuits. Afterwards if all segments are
good the status will be `ok`, if all segments are bad the status will be `error` otherwise the
status is `partial_error`.

Brake light diagnosis is currently disabled.
