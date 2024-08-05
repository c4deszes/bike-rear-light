Communication component design
==============================

The communication handler is responsible for:

* Setting up USART settings
* Initializing the LINE protocol stack
* Passing data from USART to LINE data handlers
* Updating signals
* Detecting loss of communication

Initialization
--------------

Communication is initialized based on the network specification.

Operation
---------

* PHY is updated every 1ms
* Signals are updated every 10ms
