Connector failure modes
=======================

.. fmea:: Power pin failure
    :id: FMEA_POWER_PIN_FAILURE
    :status: draft
    :probability: occasional
    :severity: critical
    :detection: always
    :mitigation: none

    Power pin failure is usually caused by contact wear, the insulator being damaged or the
    connector coming loose.

    Detection needs to be done by the cycle computer, indicated by lack of communication or a
    short/open on the battery lines.

.. fmea:: Communication pin failure
    :id: FMEA_COMMUNICATION_PIN_FAILURE
    :status: draft
    :probability: occasional
    :severity: critical
    :detection: always
    :mitigation: safety on light mode

    Communication pin failure is usually caused by contact wear, the insulator being damaged or the
    connector coming loose.

    Detection needs to be done by the cycle computer, indicated by lack of communication. The ECU
    itself should also detect the lack of communication and set it's tail light to the set safety
    brightness, signal lights in this case should remain off. Brake light can keep working as long
    as the intertial data is available.
