Parameters
==========

Device state
------------

.. param-sw:: System initialization time
    :id: PARAM_SW_SYSTEM_INIT_TIME
    :variable: uint16
    :unit: milliseconds
    :control: build

    Controls how long the device stays in initialization mode (maximum brightness)

LED control
-----------

.. param-sw:: Automatic LED diagnostics
    :id: PARAM_SW_LED_AUTO_DIAG
    :variable: bool
    :unit: true/false
    :control: build

.. param-sw:: Brightness curve X cutoff
    :id: PARAM_SW_LED_CURVE_CUTOFF_X
    :variable: uint16
    :unit: -
    :control: eeprom

.. param-sw:: Brightness curve Y cutoff
    :id: PARAM_SW_LED_CURVE_CUTOFF_Y
    :variable: uint16
    :unit: -
    :control: eeprom

.. param-sw:: Brightness curve X maximum
    :id: PARAM_SW_LED_CURVE_MAX_X
    :variable: uint16
    :unit: -
    :control: eeprom

.. param-sw:: Brightness curve Y maximum
    :id: PARAM_SW_LED_CURVE_MAX_Y
    :variable: uint16
    :unit: -
    :control: eeprom

.. param-sw:: Standard mode minimum
    :id: PARAM_SW_LED_STANDARD_MIN
    :variable: uint16
    :unit: -
    :control: eeprom

.. param-sw:: Emergency mode brightness
    :id: PARAM_SW_LED_EMERGENCY_BRIGHTNESS
    :variable: uint16
    :unit: -
    :control: eeprom

.. param-sw:: Safety mode brightness
    :id: PARAM_SW_LED_SAFETY_BRIGHTNESS
    :variable: uint16
    :unit: -
    :control: eeprom

Strobe functions
----------------

.. param-sw:: Primary strobe source
    :id: PARAM_SW_STROBE_PRIMARY_SOURCE
    :variable: enum8
    :unit: -
    :control: eeprom

.. param-sw:: Emergency strobe source
    :id: PARAM_SW_STROBE_EMERGENCY_SOURCE
    :variable: enum8
    :unit: -
    :control: eeprom

.. param-sw:: Safety strobe source
    :id: PARAM_SW_STROBE_SAFETY_SOURCE
    :variable: enum8
    :unit: -
    :control: eeprom

Communication
-------------

.. param-sw:: Enable debug signals
    :id: PARAM_SW_COMM_DEBUG_SIGNALS
    :variable: bool
    :unit: -
    :control: build

.. param-sw:: Light request timeout
    :id: PARAM_SW_COMM_LIGHT_REQUEST_TIMEOUT
    :variable: uint16
    :unit: milliseconds
    :control: build

.. param-sw:: Speed request timeout
    :id: PARAM_SW_COMM_SPEED_REQUEST_TIMEOUT
    :variable: uint16
    :unit: milliseconds
    :control: build

Brake detection
---------------

.. param-sw:: Use external brake signal
    :id: PARAM_SW_BRAKE_USE_EXTERNAL_BRAKE
    :variable: bool
    :unit: -
    :control: build

.. param-sw:: Use internal brake signal
    :id: PARAM_SW_BRAKE_USE_INTERNAL_BRAKE
    :variable: bool
    :unit: -
    :control: build
