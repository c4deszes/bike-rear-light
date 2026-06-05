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

typedef enum {
    brightness_output_tail,
    brightness_output_brake,
    brightness_output_turn
} brightness_output_t;

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
 * @brief Sets the target brightness level (0-1000)
 * 
 * @param target Target brightness level
 */
void BRIGHTNESS_SetTarget(uint16_t target);

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
 * @brief Gets the current target brightness level for a specific output
 * 
 * @param output The brightness output to get the target for
 * @return The current target brightness level for the specified output
 */
uint16_t BRIGHTNESS_GetOutput(brightness_output_t output);

/**
 * @brief Updates the brightness module, this should be called periodically
 */
void BRIGHTNESS_Update10ms(void);

#endif // APP_BRIGHTNESS_H_
