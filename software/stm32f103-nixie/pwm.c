#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#include <system.h>

#define PWM PA0

static void set_pwm_pin(void)
{
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
        GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
        GPIO_TIM2_CH1_ETR);
}

void set_power(int x)
{
    if (x < 0)
    {
        x = 0;
    }

    if (x > 100)
    {
        x = 100;
    }

    TIM2_CCR1 = 100 + x;
}

static void tim_setup(void)
{
    rcc_periph_reset_pulse(RST_TIM2);

    /* pre-scaller */ 
    TIM2_PSC = 720;

    /* frequency determined by  TIMx_ARR register (auto reload register) */
    TIM2_ARR = 200;

    /* a duty cycle determined by the value of the TIMx_CCRx register */
    /* Capture compare value */

    // 100 - 200  is working distance, 100 - is zero, 0 is full throttle
    // TIM2_CCR1 = 100 + power (%)
    TIM2_CCR1 = 150;
    TIM2_CCR2 = 0;

    /* ARR reload enable */
    TIM2_CR1 |= TIM_CR1_ARPE;
    /* Output compare 1 mode */
    /* Output compare 1 mode and preload */
    TIM2_CCMR1 |= TIM_CCMR1_OC1M_PWM1 | TIM_CCMR1_OC1PE;

    /* Polarity and state */
    TIM2_CCER |= TIM_CCER_CC1P | TIM_CCER_CC1E;
    //TIM2_CCER |= TIM_CCER_CC1E;

    /* ---- */

    /* Counter enable */
    TIM2_CR1 |= TIM_CR1_CEN;


}
/* TIM2_CH1 */
static void setup_timer(void)
{
    rcc_periph_reset_pulse(RST_TIM2);

    /* Timer global mode:
     * - No divider
     * - Alignment edge
     * - Direction up
     */
    timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT,
                   TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

    /* Reset prescaler value. */
    timer_set_prescaler(TIM2, 0);

    /* Reset repetition counter value. */
    timer_set_repetition_counter(TIM2, 500);

    /* Enable preload. */
    timer_enable_preload(TIM1);

    /* Period (500 Hz). */
    timer_set_period(TIM1, 72000000 / 1000);


    timer_set_deadtime(TIM2, 10);
    timer_set_enabled_off_state_in_idle_mode(TIM2);
    timer_set_enabled_off_state_in_run_mode(TIM2);
    timer_disable_break(TIM2);
    timer_set_break_polarity_high(TIM2);
    timer_disable_break_automatic_output(TIM2);
    timer_set_break_lock(TIM2, TIM_BDTR_LOCK_OFF);


    /* Disable outputs. */
    timer_disable_oc_output(TIM2, TIM_OC1);
    timer_disable_oc_output(TIM2, TIM_OC1N);


    /* Configure global mode of line 1. */
    timer_disable_oc_clear(TIM2, TIM_OC1);
    timer_enable_oc_preload(TIM2, TIM_OC1);
    timer_set_oc_slow_mode(TIM2, TIM_OC1);
    timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_PWM1);

    /* Configure OC1. */
    timer_set_oc_polarity_high(TIM2, TIM_OC1);
    timer_set_oc_idle_state_set(TIM2, TIM_OC1);

    /* Configure OC1N. */
    timer_set_oc_polarity_high(TIM2, TIM_OC1N);
    timer_set_oc_idle_state_set(TIM2, TIM_OC1N);


    /* Set the capture compare value for OC1. */
    timer_set_oc_value(TIM2, TIM_OC1, 500);

    /* Reenable outputs. */
    timer_enable_oc_output(TIM2, TIM_OC1);
    timer_enable_oc_output(TIM2, TIM_OC1N);

    /* ARR reload enable. */
    timer_enable_preload(TIM1);

    /*
     * Enable preload of complementary channel configurations and
     * update on COM event.
     */
    timer_enable_preload_complementry_enable_bits(TIM2);

    /* Enable outputs in the break subsystem. */
    timer_enable_break_main_output(TIM2);

    /* Counter enable. */
    timer_enable_counter(TIM2);

}
void pwm_setup(void)
{
    rcc_periph_clock_enable(RCC_TIM2);
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_AFIO);
    set_pwm_pin();
    //setup_timer();
    tim_setup();

}
