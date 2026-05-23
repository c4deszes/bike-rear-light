# Board configurations

This folder includes board configurations for all released hardware versions.

Each board configuration must define the `bsp-config` interface target.
The target must have the following properties:
- Include folder
- BSP_IMU_DRIVER, defines the IMU driver to be used
- BSP_LED_DRIVER, defines the LED driver to be used

## IMU driver

Board configs currently can use `NONE` or `BMA456`. Whether the sensor is actually present is up
to the driver to detect.

## LED driver

Board configs currently can use `TLD2xxx`, `ILD8150`, `TLD509x` or `AL8891`.
