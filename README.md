# Rear Light

> An integrated tail, brake and turn signal light for bicycles.

## Gen 1

This generation is meant to demonstrate the tail light and brake light capabilities, turn signals
are not present.

The expected takeaways from it are:

- whether this simplified single PCB assembly is robust enough
- whether the amount of light produced is sufficient
- how certain lenses affect the optical qualities 
- what the thermals look like given the power consumption and enclosure with passive cooling
- whether this accelerometer is reliable to be used for the brake lights

Key findings:

- Accelerometer alone is not reliable for brake detection due to road vibrations. With filtering
  the reaction time goes up.
- Linear LED drivers are not efficient for the expected bus voltages, this is a problem for the
  continously on tail light
- Single LEDs spread out on a large area need lots of diffusing which reduces the light output

## Gen 1b

This generation is meant as a patch for the weak and uneven light output of Gen1. It's based on the
Gen1 front light hardware put inside a custom case.

There are no expectations.

Key findings:

- Mounting bracket broke during two incidents in an unservicable way

## Gen 2

This generation is meant to merge the good parts of Gen1 and Gen1b and improve a couple of things.
The main new feature is the included radar and the primary improvements are increased operational
voltage range and additional monitoring.


