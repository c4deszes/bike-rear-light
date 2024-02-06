Light design
============

Layout
------

Fault management
----------------

.. sys-drs:: Failure reactions
    :id: SYS_DRS_LIGHT_FAILURE_REACTIONS
    :status: draft

    On POR all segments should slowly turn on, this shall be achieved by hardware measures.

    With software failures (e.g.: not booting, watchdog loop) the lights would remain on, body
    computer detects rear light issue.

    After software initialization it shall take control and depending on system conditions it
    may switch off the lights.

    With communication failure the the lights would remain at safety brightness.

    With the lights shorted/open the software reports the rear light issue.
