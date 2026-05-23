#ifndef BSP_TLD509X_CFG_H_
#define BSP_TLD509X_CFG_H_

#define TLD509x_MODULATION_TYPE_PWM 0
#define TLD509x_MODULATION_TYPE_ANALOG 1

#define TLD509x_HARDWARE_NOINIT 0
#define TLD509x_HARDWARE_MAX_PWM 1
#define TLD509x_HARDWARE_ANALOG 2

#define TLD509x_HARDWARE_TYPE TLD509x_HARDWARE_NOINIT
#define TLD509x_MODULATION_TYPE TLD509x_MODULATION_TYPE_ANALOG

#define TLD509x_TURN_OFF_DELAY_MS 15           /* Disable delay = ~10ms (increased to 15ms) */
#define TLD509x_TURN_ON_DELAY_MS 1             /* Enable startup time = 100us (rounded to 1ms) */

#define TLD509x_PWM_FREQUENCY 250u             /* PWM frequency in Hz */
#define TLD509x_PWM_MINIMUM_ON_US 10           /* Minimum on time in microseconds */

#define TLD509x_DISABLE_AT_ZERO 1              /* If set to 1, the driver will be disabled when brightness is set to 0. If set to 0, the driver will remain enabled at 0 brightness. */
#define TLD509x_DISABLE_TIMEOUT_MS 2000        /* Driver will be disabled after this timeout when brightness is set to 0 */

#endif // BSP_TLD509X_CFG_H_
