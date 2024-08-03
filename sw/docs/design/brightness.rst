Brightness component design
===========================

Brightness controller manages the target brightness for the LED segments.

Initialization
--------------

The controller starts in maximum brightness mode, in this mode it's actually the hardware
measures that enable maximum brightness.

Configuration
-------------

* Brightness curve: how the target brightness is mapped to the segments PWM control duty cycle.
* Standard minimum brightness: minimum brightness in daytime running mode
* Brake offset: target brightness increased when braking
* Strobe high/low offset: target brightness increased and decreased when strobing

Operation
---------

After initialization the controller follows the mode set by the system state manager.

* Off: all light elements are off
* Standard: following the brightness curve with an added minimum brightness, brake lights are enabled
* Adaptive: following the brightness curve, brake lights are enabled
* Emergency: fixed brightness and strobe mode
* Safety: fixed brightness and strobe mode
* Max: maximum brightness (hardware default)

Brightness curve
----------------

.. plot::

    import matplotlib.pyplot as plt
    import numpy as np
    cutoff_x = 100
    cutoff_y = 100
    max_x = 900
    max_y = 900
    min_level = 400
    emergency_level = 300
    safety_level = 800

    # Create the plot
    plt.figure(figsize=(10, 6))

    # Plot the cutoff and maximum points
    crossover_x = (min_level - cutoff_y) * (max_x - cutoff_x) / (max_y - cutoff_y) + cutoff_x
    plt.plot([0, cutoff_x, cutoff_x, max_x, 1000], [0, 0, cutoff_y, max_y, max_y], 'bo-', color='green', label='Adaptive')
    plt.plot([0, crossover_x, max_x, 1000], [min_level, min_level, max_y, max_y], 'bo-', color='blue', label='Standard')

    # Add horizontal lines for fixed brightness levels
    plt.axhline(y=emergency_level, color='green', linestyle='--', label='Emergency')
    plt.axhline(y=safety_level, color='orange', linestyle='--', label='Safety')

    # Add labels and title
    plt.xlabel('Target Brightness')
    plt.ylabel('PWM Duty Cycle')
    plt.title('Brightness to PWM Duty Cycle')
    plt.legend()

    # Show the plot
    plt.grid(True)
    plt.show()
