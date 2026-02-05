#if !defined(APP_CONFIG_H_)
#define APP_CONFIG_H_

/* --------------- Features ------------------- */
#define FEATURE_SYSTEM_TIME_INIT 1000

#define FEATURE_LED_DRIVER_STARTUP_DELAY 500u        // Time to wait before enabling the driver
#define FEATURE_LED_DRIVER_ENABLE_DELAY 100u         // Time to wait before driving the LED

#define FEATURE_COMM_ENABLE_DEBUG_SIGNALS 1
#define FEATURE_COMM_LIGHTREQUEST_TIMEOUT 4000
#define FEATURE_COMM_SPEEDSTATUS_TIMEOUT 500

#define FEATURE_LED_DRIVER_PWM_FREQUENCY 1000u

/* --------------- Settings ------------------- */
// Single strobe frequency: 6.25Hz
#define CONFIG_BRIGHTNESS_STROBE_SINGLE_ON_TIME 160
#define CONFIG_BRIGHTNESS_STROBE_SINGLE_OFF_TIME 160

// Rapid strobe pattern _______/¨\_/¨\_/¨\__
#define CONFIG_BRIGHTNESS_STROBE_RAPID_OFF_TIME 200
#define CONFIG_BRIGHTNESS_STROBE_RAPID_SWITCH_TIME 60

#endif // APP_CONFIG_H_
