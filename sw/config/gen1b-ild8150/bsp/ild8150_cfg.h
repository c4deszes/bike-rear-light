#ifndef BSP_ILD8150_CFG_H
#define BSP_ILD8150_CFG_H

#define ILD8150_HARDWARE_NOINIT 0
#define ILD8150_HARDWARE_OFF 1
#define ILD8150_HARDWARE_MAX_PWM 2

#define ILD8150_HARDWARE_TYPE ILD8150_HARDWARE_NOINIT

#define ILD8150_TURN_OFF_DELAY_MS 1           /* Disable delay = ~10ms (increased to 15ms) */
#define ILD8150_TURN_ON_DELAY_MS 1            /* Enable delay = ~10ms (increased to 15ms) */

#define ILD8150_PWM_FREQUENCY 250u             /* PWM frequency in Hz */
#define ILD8150_PWM_MINIMUM_ON_US 10           /* Minimum on time in microseconds */

#define ILD8150_DISABLE_AT_ZERO 1              /* If set to 1, the driver will be disabled when brightness is set to 0. If set to 0, the driver will remain enabled at 0 brightness. */
#define ILD8150_DISABLE_TIMEOUT_MS 2000        /* Driver will be disabled after this timeout when brightness is set to 0 */

#endif // BSP_ILD8150_CFG_H
