Light failure modes
===================

Tail light
----------

.. fmea:: Tail light failure
    :id: FMEA_TAIL_LIGHT_FAILURE
    :status: draft
    :probability: occasional
    :severity: catastrophic
    :detection: high
    :mitigation: multichannel layout

    Light failure could be caused by:

    * Shorted LEDs
    * Open LEDs
    * LED driver failure

    Mitigation should include hardware and software redundancy to ensure that the light doesn't
    turn off when it should be on.

    * Hardware should provide a default-on behavior, so the light is on even with certain software
      faults

    And obviously detection from hardware side should be maximized, should be able to detect shorts
    and opens, the rider may not detect the failure if it's not indicated by the cycle computer.

Brake light
-----------

.. fmea:: Brake light failure (off)
    :id: FMEA_BRAKE_LIGHT_FAILURE_OFF
    :status: draft
    :probability: occasional
    :severity: minor
    :detection: low
    :mitigation: increased hardware fault detection

    Light failure could be caused by:

    * Shorted LEDs
    * Open LEDs
    * LED driver failure

    Rider shall be notified in case of failure, but this failure is not severe as it's not a legal
    requirement to have working brake lights on bicycles.

.. fmea:: Brake light failure (on/switching)
    :id: FMEA_BRAKE_LIGHT_FAILURE_ON
    :status: draft
    :probability: occasional
    :severity: critical
    :detection: low
    :mitigation: software verification, protection of the control pins

    Failure could be caused by:

    * Software fault
    * Invalid data coming from the accelerometer
    * Short/open on the driver's control pins

    The mitigation should include:

    * Validation and filtering of the accelerometer data
    * Software limit on how often and how many times the brake light can turn on
    * Protection of the driver's control pins to ensure a default off behavior

    The brake light turning on when not actually braking can confuse the drivers behind.

Signal light
------------

.. fmea:: Signal light failure
    :id: FMEA_SIGNAL_LIGHT_FAILURE
    :status: draft
    :probability: occasional
    :severity: minor
    :detection: low
    :mitigation: increased hardware fault detection

    Light failure could be caused by:

    * Shorted LEDs
    * Open LEDs
    * LED driver failure

    Mitigation should include hardware and software fault detection.

    * Hardware should provide a default-off behavior, so the light is off even with software
      failures

    The rider may not detect the failure if it's not indicated by the cycle computer.
    One side blinking while the other cannot is fine, the rider should still be notified so they
    could revert back to using hand signals during turns.
