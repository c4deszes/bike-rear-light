Light requirements
==================

Positioning and size
--------------------

.. req-sys:: Tail light position
    :id: SYS_REQ_TAIL_LIGHT_POSITION
    :status: draft
    :tags: legal, gen1.0

    The tail light shall be no more than 1.5m above ground level.

Tail light
----------

.. req-sys:: Tail light performance
    :id: SYS_REQ_TAIL_LIGHT_PERFORMANCE
    :status: draft
    :tags: legal, gen1.0

    The tail light shall be the color red, with a minimum brightness of 10 candela and a maximum
    brightness of 200 candela.

    The tail light shall be visible from 1.5km in good visibility conditions and 50m in poor
    visibility conditions.

.. req-sys:: Tail light modes
    :id: SYS_REQ_TAIL_LIGHT_MODES
    :status: draft
    :tags: legal, gen1.0

    The tail light shall be able to emit light continuously or blink at a rate of 60Hz.

Brake light
-----------

.. req-sys:: Brake light
    :id: SYS_REQ_BRAKE_LIGHT_PERFORMANCE
    :status: draft
    :tags: legal, safety

    The brake light shall be the color red, with a minimum brightness of 10 candela and a maximum
    brightness of 400 candela.

    The brake light shall turn on when the bicycle is decelerating with ``5 m/s^2`` and above.

Signal lights
-------------

.. req-sys:: Signal light performance
    :id: SYS_REQ_SIGNAL_LIGHT_PERFORMANCE
    :status: draft
    :tags: legal

    The signal lights shall be the color white-yellow, yellow, amber or orange, with a minimum
    brightness same as the tail light, the signal lights shall be clearly distinguishable from the
    tail light.

.. req-sys:: Signal light modes
    :id: SYS_REQ_SIGNAL_LIGHT_MODES
    :status: draft
    
    The signal lights shall be able to blink at a rate of 60Hz, the lights can also be sequentially
    animated, in this case all the lights need be to turned on before turning the anything off.

Light control
-------------

.. req-sys:: Light control
    :id: SYS_REQ_LIGHT_CONTROL
    :status: draft

    All light elements shall be synchronized in their switching frequency that's in the visible
    range (``120Hz - 1000Hz``).
