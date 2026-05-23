#if !defined(APP_SYS_STATE_H_)
#define APP_SYS_STATE_H_

/**
 * @brief Initializes the system state module, this should be called once at startup
 */
void SYSSTATE_Init(void);

/**
 * @brief Loads the system state settings from the configuration manager
 */
void SYSSTATE_LoadConfig(void);

/**
 * @brief Updates the system state module, this should be called every 10ms
 */
void SYSSTATE_Update10ms(void);

#endif // APP_SYS_STATE_H_
