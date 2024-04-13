#if !defined(APP_BRAKE_H_)
#define APP_BRAKE_H_

typedef enum {
    brake_signal_status_na,
    brake_signal_status_ok,
    brake_signal_status_perm_error
} brake_signal_status_t;

void BRAKE_Init(void);

void BRAKE_Update10ms(void);

#endif // APP_BRAKE_H_
