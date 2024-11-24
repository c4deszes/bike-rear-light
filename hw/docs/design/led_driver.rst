LED driver design
=================

.. jupyter-execute::
    :hide-code:

    %config InlineBackend.figure_format = 'svg'
    import numpy as np
    from matplotlib import pyplot
    from IPython.display import Latex
    from UliEngineering.Electronics.Resistors import normalize_numeric

Challenges
----------

* Minimizing footprint and component number while including redundancy in the design
* Keeping the device efficient in a wide operating voltage range

Conclusions
-----------

* 2 channel drivers don't make sense if redundancy is important due to package size constraints
* To minimize footprint the optimal driver is either:

  * single channel and has boost capabilities (8 LEDs in a string, roughly Vf=16V)
  * quad channel and has buck capabilities (4x2 LED strings, roughly Vf=4V)

* Current source type drivers can only be used at low voltages, they would cutout if the string's
  forward voltage was too high

LED string design
-----------------

Given a nominal battery voltage of ``7.4V`` the longest LED string would be 3 LEDs with an average
forward voltage drop of ``2.5V``.

Brake & tail light
------------------

The chosen drivers are of the TLD2xxx series.

To achieve low power consumption the EN/DEN pin of the chip can be pulled low.

The driver is able to detect the following scenarios:

* Open load
* Short circuit
* Single LED short circuit

In case of open and shorted outputs the driver's reaction is determined by the D pin:

* When shorted to the ground the other channels will remain on
* When connected to a capacitor the capacitance determines the time after which channels are turned
  off

In case of a single LED short the DS pin does the same as the D pin but when it's shorted to ground
the output remains on with the shorted LED and with the capacitor it turns the channel off, the
capacitance in this case determines how often it tries to turn the channel on again.

The detection of single LED shorts is up to the proper sizing of the SLS resistor, when it's a short
to ground the feature is disabled, outputs will remain on.

In any of the above cases the ERROR pin is pulled low.

Detecting which channel has an error can be done by sampling the voltage on the INSET pins.

Configuration
~~~~~~~~~~~~~

* EN/DEN is pulled high to comply with the safety requirements,
  the microcontroller can then pull it low when lights are not needed.

* ERROR pin is connected to the microcontroller to report back errors.

* D/DS is shorted to ground, channels should remain on for as long as possible

* SLS is not used, as the variances in the forward voltage would not allow us to reliable detect
  this scenario.

* INSET is connected to transistors


Calculation
~~~~~~~~~~~

Calculation starts with determining the Input Set resistor value.

.. jupyter-execute::
    :hide-code:

    v_inset_ref = '1.22V'
    k = 300
    i_out = '20mA'

    r_inset = (k * normalize_numeric(v_inset_ref)) / normalize_numeric(i_out)

    Latex(f'R_{{INSETx}} = {r_inset:.03f}Ω')

The lights have to be operational in low voltage conditions, the lowest voltage devices should
expect is ``6.5V``. Since the device is a current source there's going to be a voltage drop across
it which then heats the device.


Signal light
------------

Turn indication is not implemented in Gen1, but here are few possible choices for it's driver:

* TLC6C5816-Q1 : 16 channel, on/off with group dimming?, no fault feedback, open drain, needs series resistor
* TLC6A598: 8 channel, on/off with dimming, no fault feedback
* TLC5917-Q1: 8 bit, shift register, prevents shorts
* TLC59208F: 8 bit, needs series resistor, i2c control with dimming per channel, no fault feedback
* TLC69601-Q1: 16 bit, i2c, fault detection, no further info, has many variants
* TPS92391: 6 channel, i2c, qfn32, boost/sepic with current sink
* LP8862-Q1: 2 channel, pwm (single channel), fault detection
* TPS92638-Q1: 8 channel, pwm (four channel), current source
* TLD7002-16ES: 16 channel, current source, fault detection, specifically for animations
