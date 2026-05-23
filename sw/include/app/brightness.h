#if !defined(APP_BRIGHTNESS_H_)
#define APP_BRIGHTNESS_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    brightness_mode_off,
    brightness_mode_standard,
    brightness_mode_adaptive,
    brightness_mode_emergency,
    brightness_mode_safety,
    brightness_mode_max
} brightness_mode_t;

/**
 * @brief Initializes the brightness module, this should be called once at startup
 */
void BRIGHTNESS_Init(void);

/**
 * @brief Loads the brightness settings from the configuration manager
 */
void BRIGHTNESS_LoadConfig(void);

/**
 * @brief Sets the brightness mode
 * 
 * @param mode The brightness mode to set
 */
void BRIGHTNESS_SetMode(brightness_mode_t mode);

/**
 * @brief Gets the current brightness mode
 * 
 * @return The current brightness mode
 */
brightness_mode_t BRIGHTNESS_GetMode(void);

/**
 * @brief Sets the target brightness level (0-1000)
 * 
 * @param target Target brightness level
 */
void BRIGHTNESS_SetTarget(uint16_t target);

/**
 * @brief Gets the current target brightness level
 * 
 * @return The current target brightness level
 */
uint16_t BRIGHTNESS_GetTarget(void);

/**
 * @brief Sets the brake light state
 * 
 * @param brake Brake light is active or not
 */
void BRIGHTNESS_SetBraking(bool brake);

/**
 * @brief Transfers the strobe signal to the brightness controller
 * 
 * @param strobe Strobe is on or off
 */
void BRIGHTNESS_SetStrobe(bool strobe);

/**
 * @brief Updates the brightness module, this should be called periodically
 */
void BRIGHTNESS_Update10ms(void);

#endif // APP_BRIGHTNESS_H_
