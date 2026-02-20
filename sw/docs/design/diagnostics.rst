Diagnostic components design
============================

Dianostic component is responsible for all diagnostic request handling, including:

* Software version reporting
* Operation status reporting
* Power status reporting
* Bootloader entry
* Idle and shutdown entry

Initialization
--------------

* Mode entry flags are cleared
* Operation status is set to ``init``

Operation
---------

* Operation status is updated, based on the light status
* Power status is updated, including voltage and current measurements
