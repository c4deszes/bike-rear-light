#if !defined(APP_FEATURE_H_)
#define APP_FEATURE_H_

/* --------------- Features ------------------- */

/* System feature */
#define FEATURE_SYSTEM_TIME_INIT 1000

/* Configuration feature */
#define FEATURE_CONFIG_LOAD_AT_STARTUP 0        // When enabled, configuration is loaded from NVRAM at startup
#define FEATURE_CONFIG_RELOAD_ON_CHANGE 0       // When enabled, configuration is reloaded as soon as a property is changed
#define FEATURE_CONFIG_SAVE_AT_SHUTDOWN 0       // When enabled, configuration is saved to NVRAM at shutdown

/* Brake feature */
#define FEATURE_BRAKE_ENABLE_SENSOR 1
#define FEATURE_BRAKE_USE_EXTERNAL_SIGNAL 1
#define FEATURE_BRAKE_USE_INTERNAL_SIGNAL 1

#define FEATURE_BRAKE_HOLDOFF_TIME 100       // After braking the brake lights are not activated for this long
#define FEATURE_BRAKE_HOLDON_TIME 500        // After braking the brake lights are kept active for this long
#define FEATURE_BRAKE_MINIMUM_TIME 30        // The internal braking signal must be active for this long before lights are turned on

/* Communication feature */
#define FEATURE_COMM_ENABLE_DEBUG_SIGNALS 1
#define FEATURE_COMM_LIGHTREQUEST_TIMEOUT 4000
#define FEATURE_COMM_SPEEDSTATUS_TIMEOUT 500

#define FEATURE_DIAG_ENABLE_BOOTENTRY 1
#define FEATURE_DIAG_ENABLE_UDS_RELOAD 1
#define FEATURE_DIAG_ENABLE_UDS_SAVE 1

/* Strobe feature */
#define FEATURE_STROBE_RAPID_CYCLES 5

/* Brightness feature */
#define FEATURE_BRIGHTNESS_BRAKE_IN_OFF_MODE 1
#define FEATURE_BRIGHTNESS_BRAKE_IN_SAFETY_MODE 1
#define FEATURE_BRIGHTNESS_BRAKE_IN_EMERGENCY_MODE 0

/* LED driver feature */
#define FEATURE_LED_DRIVER_PWM_FREQUENCY 1000u
#define FEATURE_LED_DRIVER_STARTUP_DELAY 500u        // Time to wait before enabling the driver
#define FEATURE_LED_DRIVER_ENABLE_DELAY 100u         // Time to wait before driving the LED

#endif // APP_FEATURE_H_
