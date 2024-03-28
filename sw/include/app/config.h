#if !defined(APP_CONFIG_H_)
#define APP_CONFIG_H_

#define CONFIG_BRIGHTNESS_STANDARD_MIN_LEVEL 20
#define CONFIG_BRIGHTNESS_ADAPTIVE_CUTOFF 10
#define CONFIG_BRIGHTNESS_ADAPTIVE_CUTOFF_LEVEL 20
// TODO: further curve points
#define CONFIG_BRIGHTNESS_EMERGENCY_LEVEL 30
#define CONFIG_BRIGHTNESS_SAFETY_LEVEL 80

// TODO: when blinking should we use an offset from the brightness curve or a fixed low level?
#define CONFIG_BRIGHTNESS_STROBE_LOW_LEVEL 0

#endif // APP_CONFIG_H_
