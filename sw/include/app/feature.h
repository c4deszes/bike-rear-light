#if !defined(APP_FEATURE_H_)
#define APP_FEATURE_H_

/* --------------- Features ------------------- */
#define FEATURE_SYSTEM_TIME_INIT 1000

#define FEATURE_BRAKE_ENABLE_SENSOR 0
#define FEATURE_BRAKE_USE_EXTERNAL_SIGNAL 1
#define FEATURE_BRAKE_USE_INTERNAL_SIGNAL 0

#define FEATURE_COMM_ENABLE_DEBUG_SIGNALS 1
#define FEATURE_COMM_LIGHTREQUEST_TIMEOUT 4000
#define FEATURE_COMM_SPEEDSTATUS_TIMEOUT 500

#define FEATURE_LED_DRIVER_PWM_FREQUENCY 1000u
#define FEATURE_LED_DRIVER_STARTUP_DELAY 500u        // Time to wait before enabling the driver
#define FEATURE_LED_DRIVER_ENABLE_DELAY 100u         // Time to wait before driving the LED

#define CONFIG_BRAKE_HOLDOFF_TIME 100       // After braking the brake lights are not activated for this long
#define CONFIG_BRAKE_HOLDON_TIME 500        // After braking the brake lights are kept active for this long
#define CONFIG_BRAKE_MINIMUM_TIME 30        // The internal braking signal must be active for this long before lights are turned on

#endif // APP_FEATURE_H_
