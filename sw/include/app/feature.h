#if !defined(APP_FEATURE_H_)
#define APP_FEATURE_H_

/* --------------- Features ------------------- */

/* System feature */
#define FEATURE_SYSTEM_TIME_INIT 1000

/* Configuration feature */
#define FEATURE_CONFIG_LOAD_AT_STARTUP 1        // When enabled, configuration is loaded from NVRAM at startup
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

/* Calibration feature */
#define FEATURE_CALIB_ENABLE 1
#define FEATURE_CALIB_LOAD_AT_STARTUP 1

/* LED driver feature */
#define FEATURE_LED_DRIVER_PWM_FREQUENCY 1000u
#define FEATURE_LED_DRIVER_STARTUP_DELAY 500u        // Time to wait before enabling the driver
#define FEATURE_LED_DRIVER_ENABLE_DELAY 100u         // Time to wait before driving the LED

/* Temperature feature */
#define FEATURE_TEMP_DRIVE_TEMP_TIMEOUT 200        // Number of consecutive failed readings before considering drive temperature measurement as failed
#define FEATURE_TEMP_MCU_TEMP_TIMEOUT 200          // Number of consecutive failed readings before considering MCU temperature measurement as failed

#define FEATURE_TEMP_DRIVE_MAX_TEMPERATURE 90
#define FEATURE_TEMP_DRIVE_MIN_TEMPERATURE -20

#define FEATURE_TEMP_DRIVE_FACTOR 35
#define FEATURE_TEMP_MCU_FACTOR 5

/* Voltage feature */
#define FEATURE_VOLT_VBAT_TIMEOUT 200        // Number of consecutive failed readings before considering drive voltage measurement as failed
#define FEATURE_VOLT_VBAT_LOW_THRESHOLD 60   // Voltage level in 100mV [6.0V]

/* Current feature */
#define FEATURE_CURRENT_VOLT_DERATE_X 0, 60, 90, 150, 180, 240
#define FEATURE_CURRENT_VOLT_DERATE_Y 0, 0, 1000, 1000, 0, 0

#define FEATURE_CURRENT_TEMP_DERATE_X -20, 0, 50, 70, 90
#define FEATURE_CURRENT_TEMP_DERATE_Y 500, 1000, 1000, 300, 0

#endif // APP_FEATURE_H_
