Mechanical failure modes
========================

.. fmea:: Water ingress
    :id: FMEA_WATER_INGRESS
    :status: draft
    :probability: occasional
    :severity: critical
    :detection: low
    :mitigation: sealant

    Severity depends on the amount of water and the time the water has been in the system.
    Over time water corrodes parts, causes shorts or open circuits.

    The hardware should have mechanisms that indicate these shorts early and the software should
    report them back to the cycle computer as well as keeping track of them over time.

.. fmea:: Mount failure
    :id: FMEA_MOUNT_FAILURE
    :status: draft
    :probability: remote
    :severity: catastrophic
    :detection: high
    :mitigation: redundant mounting mechanism, cable tie down

    With the mount coming loose the rear light would slide down the seatpost, this is likely
    detected by the user.

    With the mount completely failing there's a risk of cable entanglement in the rear wheel.
    The cable therefore should be tied down at multiple points. The rear wheel being stuck would
    lead to an uncontrolled deceleration, the bike would likely be manuverable during it but the
    rear wheel would slip.
