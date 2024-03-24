Controller
==========

The device uses the ATSAMD21E18A microcontroller, below are the pin assigments.

* Tail and brake light elements all have their separate IO, in future versions some pins could be
  made common (PWM, Enable)
* Accelerometer is used in SPI mode and both interrupt pins are connected, the number of pins used
  could be reduced by using I2C mode and not using interrupts

Pinout
------

.. list-table:: Pinout
    :header-rows: 1

    * - Pin
      - Pad
      - Peripheral
      - Function
      - Description

    * - PA00
      - 1
      - XTAL32
      - XIN32
      - 32kHz crystal input

    * - PA01
      - 2
      - XTAL32
      - XOUT32
      - 32kHz crystal output

    * - PA02
      - 3
      - EIC
      - EXTINT[2]
      - Tail light Error

    * - PA03
      - 4
      - EIC
      - EXTINT[3]
      - Brake light Error

    * - PA04
      - 5
      - TCC3
      - WO[2]
      - Tail light segment 3

    * - PA05
      - 6
      - TCC3
      - WO[3]
      - Tail light segment 2

    * - PA06
      - 7
      - TCC3
      - WO[4]
      - Tail light segment 1

    * - PA07
      - 8
      - TCC3
      - WO[5]
      - Brake light segment 1

    * - PA08
      - 11
      - GPIO
      - GPIO[8]
      - Tail light enable

    * - PA09
      - 12
      - GPIO
      - GPIO[9]
      - Brake light enable

    * - PA10
      - 13
      - TCC1
      - WO[0]
      - Tail light PWM control

    * - PA11
      - 14
      - TCC1
      - WO[1]
      - Brake light PWM control

    * - PA14
      - 15
      - EIC
      - EXTINT[14]
      - Accelerometer interrupt 2

    * - PA15
      - 16
      - EIC
      - EXTINT[15]
      - Accelerometer interrupt 1

    * - PA16
      - 17
      - SERCOM1
      - SERCOM PAD[0]
      - Accelerometer SPI Master In

    * - PA17
      - 18
      - SERCOM1
      - SERCOM PAD[1]
      - Accelerometer SPI Clock

    * - PA18
      - 19
      - SERCOM1
      - SERCOM PAD[2]
      - Accelerometer chip select

    * - PA19
      - 20
      - SERCOM1
      - SERCOM PAD[3]
      - Accelerometer SPI Master out

    * - PA22
      - 21
      - GPIO
      - GPIO[22]
      - LIN Transceiver Chip Select

    * - PA23
      - 22
      - GPIO
      - GPIO[23]
      - LIN Transceiver Transmit enable / Fault

    * - PA24
      - 23
      - SERCOM3
      - PAD2 (TX)
      - LIN Transceiver Transmit

    * - PA25
      - 24
      - SERCOM3
      - PAD3 (RX)
      - LIN Transceiver Receive

    * - PA30
      - 31
      - SWD
      - SWCLK
      - Serial-Wire-Debug clock

    * - PA31
      - 32
      - SWD
      - SWDIO
      - Serial-Wire-Debug data in/out
