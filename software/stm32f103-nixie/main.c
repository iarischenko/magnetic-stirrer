#include <stdlib.h>
#include <stdio.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>

/* #include <libopencm3/cm3/sync.h> */

#include <ssd1306.h>
#include <ssd1306_tests.h>

#include <PD_UFP.h>
#include <PD_UFP_Protocol.h>

#include "api.h"
#include "api-asm.h"
#include "system.h"


enum __state {
    SET_TIME,
    SET_POWER,
    RUNNING,
    SYSTEM_STATE_LAST,
};

struct system_state {
    enum __state  state;
    int power;
    int end;
    int duration;
    bool need_update;
    bool state_is_changed;
};


/* Set up a timer to create 1mS ticks. */
static void systick_setup(void)
{
    /* 72MHz / 8 => 9000000 counts per second. */
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
    /* 9000000/9000 = 1000 overflows per second - every 1ms one interrupt */
    /* SysTick interrupt every N clock pulses: set reload to N-1 */
    systick_set_reload(8999);
    systick_interrupt_enable();
    /* Start counting. */
    systick_counter_enable();
}

uint32_t system_millis = 0;
/* Called when systick fires */
void sys_tick_handler(void);
void sys_tick_handler(void)
{
    system_millis++;
}

static struct system_state *get_system_state(void)
{
    static struct system_state state = {
        .state = SET_TIME,
        .power = 5,
        .end = 0,
        .duration = 120000,
        .need_update = true,
        .state_is_changed = false,
    };
    return &state;
}

void step_to_next_state(void)
{
    struct system_state *state = get_system_state();

    if (state->state == SET_TIME)
    {
        state->state = SET_POWER;
    }
    else if (state->state == SET_POWER)
    {
        state->state = RUNNING;
        set_power(state->power);
        state->end = system_millis + state->duration;
    }
    else
    {
        set_power(0);
        state->state = SET_TIME;
    }

    state->state_is_changed = true;
}

void adjust_number(int i)
{
    struct system_state *state = get_system_state();

    if (state->state == SET_TIME)
    {
        state->duration += 1000 * i;
        if (state->duration < 0)
        {
            state->duration = 0;
        }
    }

    if (state->state == SET_POWER || state->state == RUNNING)
    {
        state->power += i;
        state->power = (state->power < 0) ? 0 : state->power;
        state->power = (state->power > 100) ? 100 : state->power;

        if (state->state == RUNNING)
        {
           set_power(state->power); 
        }
    }
    
    state->need_update = true;
}

#define update_diplay(color1, color2)                           \
    sec = (( duration / 1000) % 60);                            \
    min = (( duration / 1000) / 60);                            \
    snprintf(buffer, BUF_LEN, " %2d:%02d   ", min, sec);         \
    ssd1306_SetCursor(2, 0);                                    \
    ssd1306_WriteString(buffer, Font_16x26, color1);            \
    snprintf(buffer, BUF_LEN, " %3d%%    ", state->power);      \
    ssd1306_SetCursor(2, 36);                                   \
    ssd1306_WriteString(buffer, Font_16x26, color2);

#define BUF_LEN 64
static void update_system_state(void )
{
    struct system_state *state = get_system_state();
    char buffer[BUF_LEN] = { 0 };

    state->need_update = false;
    state->state_is_changed = false;

    if (state->state == SET_TIME)
    {
        int min = 0, sec = 0, duration = state->duration;
        update_diplay(Black, White);
    }
    else if (state->state == SET_POWER)
    {
        int min = 0, sec = 0, duration = state->duration;
        update_diplay(White, Black);
    }
    else
    {
        int min = 0, sec = 0, duration = state->end - system_millis;

        if (duration < 0)
        {
            step_to_next_state();
            return;
        }
        update_diplay(White, White);
    }

    ssd1306_UpdateScreen();
}

int main(void) {
    int i = 0;
    struct usb_pd_pps context = {};
    volatile int wait = 0;
    struct system_state *state = get_system_state();

    /* debug wait for connection */
    while (wait)
    {
        for (i = 0; i < 0x8000; i++)
            __asm__("nop");
    }

    /* clock */
    rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
    /* millis */
    systick_setup();
    /* logs */
    usart1_setup();

    /* setup pec16 */
    setup_pec16();

    /* rtc */
    // setup_rtc();

    pwm_setup();

    PD_init(&context, PD_POWER_OPTION_MAX_12V);
    //PD_init_PPS(&context, PPS_V(12.0), PPS_A(1), PD_POWER_OPTION_MAX_15V);
    ssd1306_Init();

    while (1) {
        PD_run(&context);
        update_system_state();
    }
}


/* 
    ssd1306_SetCursor(2, 24);
    ssd1306_WriteString("Font 16x26", Font_16x26, White);
    ssd1306_UpdateScreen();

    ssd1306_Fill(White);
    ssd1306_UpdateScreen();
    
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();

    ssd1306_SetCursor(2,0);
    ssd1306_WriteString("Testing...", Font_11x18, White);
    ssd1306_SetCursor(2, 18);
    ssd1306_WriteString("0123456789A", Font_11x18, Black);
    ssd1306_SetCursor(2, 18*2);
    ssd1306_WriteString("0123456789A", Font_11x18, White);
    ssd1306_UpdateScreen();
*/
