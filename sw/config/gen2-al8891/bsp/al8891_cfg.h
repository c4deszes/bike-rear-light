#ifndef BSP_AL8891_CFG_H_
#define BSP_AL8891_CFG_H_

#define AL8891_MODULATION_TYPE_PWM 0
#define AL8891_MODULATION_TYPE_ANALOG 1

#define AL8891_HARDWARE_NOINIT 0
#define AL8891_HARDWARE_MAX_PWM 1
#define AL8891_HARDWARE_ANALOG 2

#define AL8891_HARDWARE_TYPE AL8891_HARDWARE_ANALOG
#define AL8891_MODULATION_TYPE AL8891_MODULATION_TYPE_ANALOG

#define AL8891_TURN_OFF_DELAY_MS 1           /* Disable delay = ~10ms (increased to 15ms) */
#define AL8891_TURN_ON_DELAY_MS 1             /* Enable startup time = 100 us (rounded to 1ms) */

#define AL8891_PWM_FREQUENCY 250u             /* PWM frequency in Hz */
#define AL8891_PWM_MINIMUM_ON_US 10           /* Minimum on time in microseconds */

#define AL8891_DISABLE_AT_ZERO 1              /* If set to 1, the driver will be disabled when brightness is set to 0. If set to 0, the driver will remain enabled at 0 brightness. */
#define AL8891_DISABLE_TIMEOUT_MS 1           /* Driver will be disabled after this timeout when brightness is set to 0 */

#endif // BSP_AL8891_CFG_H_
