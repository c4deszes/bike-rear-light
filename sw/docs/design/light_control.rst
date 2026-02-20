Light control component design
==============================

Light control is responsible for setting the PWM outputs according to the target duty cycle,
it's also responsible for detecting errors and diagnosing the output stages.

Initialization
--------------

The component sets up the TCC1 peripheral to output a 1kHz PWM signal with variable duty cycle.
The outputs are inverted because the drivers use an active low signal.

Initially PWM output is set to the maximum for the taillight this conforms to the hardware default state.

Operation
---------

Component is updated every 10ms, in this call:

* PWM duty cycle is reevaluated
* Output stage errors are detected
* Diagnosis requests are handled

Diagnosis (ILD8150)
-------------------

Diagnosis (TLD509x)
-------------------

Diagnosis (TLD2xxx)
-------------------

Tail light
~~~~~~~~~~

Tail light is evaluated segment by segment, initially all segments are off then the driver cycles
through them identifying the segments with short or open circuits. Afterwards if all segments are
good the status will be `ok`, if all segments are bad the status will be `error` otherwise the
status is `partial_error`.

Below is the timing diagram showing how each segment is individually enabled and the error pin
is sampled.

.. kroki::
    :type: wavedrom

    { signal: [
        { name: "!ERR",   wave: "x1314151x", data: ["S1", "S2", "S3"] },
        { name: "ENABLE", wave: "x0101010x" },
        { name: "!PWMI",  wave: "x10....1x" },
        { name: "INSET1", wave: "x010....x" },
        { name: "INSET2", wave: "x0..10..x" },
        { name: "INSET3", wave: "x0....10x" }
    ]}

Brake light
~~~~~~~~~~~

Brake light diagnosis is currently disabled.

.. kroki::
    :type: wavedrom

    { signal: [
        { name: "!ERR",   wave: "x131x", data: ["S1"] },
        { name: "ENABLE", wave: "x010x" },
        { name: "!PWMI",  wave: "x101x" },
        { name: "INSET",  wave: "x010x" },
    ]}
