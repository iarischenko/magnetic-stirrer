#ifndef STM32_SYSTEM_HEADER
#define STM32_SYSTEM_HEADER

#include <stdbool.h>
#include <time.h>


void pwm_setup(void);
void setup_rtc(void);
void get_system_time(struct tm *current_time);
void set_system_time(struct tm *new_time);
bool *get_rtc_updated_flag(void);

void usart1_setup(void);
void setup_pec16(void);
void setup_tubes(void);
void set_tubes(int digit1, int digit2, int digit3, int digit4, int digit5, int digit6);
void tubes_refresh(void);

void step_to_next_state(void);
void adjust_number(int i);
// enum system_state *get_system_state(void);


void set_power(int x);

#endif /* STM32_SYSTEM_HEADER */
