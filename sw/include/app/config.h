#if !defined(APP_CONFIG_H_)
#define APP_CONFIG_H_

/* --------------- Features ------------------- */
#define FEATURE_SYSTEM_TIME_INIT 2500

#define FEATURE_BRAKE_USE_EXTERNAL_SIGNAL 1
#define FEATURE_BRAKE_USE_INTERNAL_SIGNAL 1

#define FEATURE_COMM_ENABLE_DEBUG_SIGNALS 1

#define FEATURE_COMM_LIGHTREQUEST_TIMEOUT 4000
#define FEATURE_COMM_SPEEDSTATUS_TIMEOUT 500

/* --------------- Settings ------------------- */
/* Brightness curve points */
#define CONFIG_BRIGHTNESS_CURVE_CUTOFF_X 100
#define CONFIG_BRIGHTNESS_CURVE_CUTOFF_Y 100
#define CONFIG_BRIGHTNESS_CURVE_MAX_X 1000
#define CONFIG_BRIGHTNESS_CURVE_MAX_Y 1000

/* Fixed brightness levels */
#define CONFIG_BRIGHTNESS_STANDARD_MIN_LEVEL 200
#define CONFIG_BRIGHTNESS_EMERGENCY_LEVEL 300
#define CONFIG_BRIGHTNESS_SAFETY_LEVEL 800

/* Brake light settings */
#define CONFIG_BRIGHTNESS_BRAKE_OFFSET 200

#define CONFIG_BRAKE_HOLDOFF_TIME 100
#define CONFIG_BRAKE_HOLDON_TIME 1000
#define CONFIG_BRAKE_MINIMUM_TIME 100

/* Strobe light settings */
// TODO: should emergency mode use different values? usually there the brightness is low
//       alternatively this could be a ratio instead of offset (e.g.: 1.0 -> CurrentTarget, 0.5 -> 50% of the target)
#define CONFIG_BRIGHTNESS_STROBE_LOW_OFFSET 1000
#define CONFIG_BRIGHTNESS_STROBE_HIGH_OFFSET 0          // note: when disabled high offset is constantly applied, so it should be 0 for most cases

// Single strobe frequency: 30Hz
#define CONFIG_BRIGHTNESS_STROBE_SINGLE_ON_TIME 160
#define CONFIG_BRIGHTNESS_STROBE_SINGLE_OFF_TIME 160

// Rapid strobe pattern _______/¨\_/¨\_/¨\__
#define CONFIG_BRIGHTNESS_STROBE_RAPID_OFF_TIME 200
#define CONFIG_BRIGHTNESS_STROBE_RAPID_SWITCH_TIME 60

#define CONFIG_STROBE_SOURCE_DISABLED 0
#define CONFIG_STROBE_SOURCE_INTERNAL_SINGLE 1
#define CONFIG_STROBE_SOURCE_INTERNAL_RAPID 2
//TODO: external positive and negative

#define CONFIG_DEFAULT_STROBE_SOURCE CONFIG_STROBE_SOURCE_DISABLED
#define CONFIG_PRIMARY_STROBE_SOURCE CONFIG_STROBE_SOURCE_INTERNAL_SINGLE
#define CONFIG_SAFETY_STROBE_SOURCE CONFIG_STROBE_SOURCE_INTERNAL_RAPID
#define CONFIG_EMERGENCY_STROBE_SOURCE CONFIG_STROBE_SOURCE_DISABLED

#endif // APP_CONFIG_H_
