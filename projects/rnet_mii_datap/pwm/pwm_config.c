/*! \file pwm_config.c
 *
 * Manage the pwm for the servo
 *
 * Attempt to standardize units on 'ticks' of PWM module
 *   One PWM clock is one 'tick'
 */

/*!
 * \defgroup pwm_config PWM Configuration for Experimenting
 * @{
 */
#include <stdlib.h>
#include <stdint.h>
#include "ch.h"
#include "hal.h"
#include "usbdetail.h"

#include "chprintf.h"

#include "pwm_lld.h"
#include "pwm_config.h"



static uint32_t      pwm_freq_hz                = INIT_PWM_FREQ;

static pwmcnt_t      pwm_period_ticks           = INIT_PWM_PERIOD_TICS;
static pwmcnt_t      pwm_pulse_width_ticks      = INIT_PWM_PULSE_WIDTH_TICS;

/*
 * Configuration structure.
 */
static PWMConfig pwmcfg = {
    INIT_PWM_FREQ,             /* 6Mhz PWM clock frequency; (1/f) = 'ticks' => 167ns/tick */
    INIT_PWM_PERIOD_TICS,      /* PWM period_ticks in ticks; (T) in ticks * us/tick = 20000 ticks * 167nS/tick = 3.34mS period =>  300 Hz  */
    NULL,                      /* No callback */
    {
            {PWM_OUTPUT_DISABLED, NULL},
            {PWM_OUTPUT_ACTIVE_HIGH, NULL},
            {PWM_OUTPUT_ACTIVE_HIGH, NULL},
            {PWM_OUTPUT_ACTIVE_HIGH, NULL},
    },
    0
};



/**
 * @brief   Configures and activates the PWM peripheral.
 * @note    Starting a driver that is already in the @p PWM_READY state
 *          disables all the active channels.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 *
 * @notapi
 */
void my_pwm_lld_start(PWMDriver *pwmp) {
//  BaseSequentialStream *chp   =  (BaseSequentialStream *)&SDU_PSAS;
  uint32_t psc;
  uint16_t ccer;


  if (pwmp->state == PWM_STOP) {

    /* Clock activation and timer reset.*/
#if STM32_PWM_USE_TIM1
    if (&PWMD1 == pwmp) {
      rccEnableTIM1(FALSE);
      rccResetTIM1();
      nvicEnableVector(STM32_TIM1_UP_NUMBER,
                       CORTEX_PRIORITY_MASK(STM32_PWM_TIM1_IRQ_PRIORITY));
      nvicEnableVector(STM32_TIM1_CC_NUMBER,
                       CORTEX_PRIORITY_MASK(STM32_PWM_TIM1_IRQ_PRIORITY));
      pwmp->clock = STM32_TIMCLK2;
    }
#endif
#if STM32_PWM_USE_TIM2
    if (&PWMD2 == pwmp) {
      rccEnableTIM2(FALSE);
      rccResetTIM2();
      nvicEnableVector(STM32_TIM2_NUMBER,
                       CORTEX_PRIORITY_MASK(STM32_PWM_TIM2_IRQ_PRIORITY));
      pwmp->clock = STM32_TIMCLK1;
    }
#endif
#if STM32_PWM_USE_TIM3
    if (&PWMD3 == pwmp) {
      rccEnableTIM3(FALSE);
      rccResetTIM3();
      nvicEnableVector(STM32_TIM3_NUMBER,
                       CORTEX_PRIORITY_MASK(STM32_PWM_TIM3_IRQ_PRIORITY));
      pwmp->clock = STM32_TIMCLK1;
    }
#endif
#if STM32_PWM_USE_TIM4
    if (&PWMD4 == pwmp) {
      rccEnableTIM4(FALSE);
      rccResetTIM4();
      nvicEnableVector(STM32_TIM4_NUMBER,
                       CORTEX_PRIORITY_MASK(STM32_PWM_TIM4_IRQ_PRIORITY));
      pwmp->clock = STM32_TIMCLK1;
    }
#endif

#if STM32_PWM_USE_TIM5
    if (&PWMD5 == pwmp) {
      rccEnableTIM5(FALSE);
      rccResetTIM5();
      nvicEnableVector(STM32_TIM5_NUMBER,
                       CORTEX_PRIORITY_MASK(STM32_PWM_TIM5_IRQ_PRIORITY));
      pwmp->clock = STM32_TIMCLK1;
    }
#endif
#if STM32_PWM_USE_TIM8
    if (&PWMD8 == pwmp) {
      rccEnableTIM8(FALSE);
      rccResetTIM8();
      nvicEnableVector(STM32_TIM8_UP_NUMBER,
                       CORTEX_PRIORITY_MASK(STM32_PWM_TIM8_IRQ_PRIORITY));
      nvicEnableVector(STM32_TIM8_CC_NUMBER,
                       CORTEX_PRIORITY_MASK(STM32_PWM_TIM8_IRQ_PRIORITY));
      pwmp->clock = STM32_TIMCLK2;
    }
#endif
#if STM32_PWM_USE_TIM9
    if (&PWMD9 == pwmp) {
      rccEnableTIM9(FALSE);
      rccResetTIM9();
      nvicEnableVector(STM32_TIM9_NUMBER,
                       CORTEX_PRIORITY_MASK(STM32_PWM_TIM9_IRQ_PRIORITY));
      pwmp->clock = STM32_TIMCLK1;
    }
#endif

    /* All channels configured in PWM1 mode with preload enabled and will
       stay that way until the driver is stopped.*/
    pwmp->tim->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 |
                       TIM_CCMR1_OC1PE |
                       TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 |
                       TIM_CCMR1_OC2PE;
    pwmp->tim->CCMR2 = TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 |
                       TIM_CCMR2_OC3PE |
                       TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2 |
                       TIM_CCMR2_OC4PE;
  }
  else {
    /* Driver re-configuration scenario, it must be stopped first.*/
    pwmp->tim->CR1    = 0;                  /* Timer disabled.              */
    pwmp->tim->DIER   = 0;                  /* All IRQs disabled.           */
    pwmp->tim->SR     = 0;                  /* Clear eventual pending IRQs. */
    pwmp->tim->CCR[0] = 0;                  /* Comparator 1 disabled.       */
    pwmp->tim->CCR[1] = 0;                  /* Comparator 2 disabled.       */
    pwmp->tim->CCR[2] = 0;                  /* Comparator 3 disabled.       */
    pwmp->tim->CCR[3] = 0;                  /* Comparator 4 disabled.       */
    pwmp->tim->CNT  = 0;                    /* Counter reset to zero.       */
  }


  /* Timer configuration.*/
  psc = (pwmp->clock / pwmp->config->frequency) - 1;

  chDbgAssert((psc <= 0xFFFF),
               "pwm_lld_start(), #1", "invalid frequency");

//  chDbgAssert((psc <= 0xFFFF) &&
//              ((psc + 1) * pwmp->config->frequency) == pwmp->clock,
//              "pwm_lld_start(), #1", "invalid frequency");

  pwmp->tim->PSC  = (uint16_t)psc;
  pwmp->tim->ARR  = (uint16_t)(pwmp->period - 1);
  pwmp->tim->CR2  = pwmp->config->cr2;

  /* Output enables and polarities setup.*/
  ccer = 0;
  switch (pwmp->config->channels[0].mode & PWM_OUTPUT_MASK) {
  case PWM_OUTPUT_ACTIVE_LOW:
    ccer |= TIM_CCER_CC1P;
  case PWM_OUTPUT_ACTIVE_HIGH:
    ccer |= TIM_CCER_CC1E;
  default:
    ;
  }
  switch (pwmp->config->channels[1].mode & PWM_OUTPUT_MASK) {
  case PWM_OUTPUT_ACTIVE_LOW:
    ccer |= TIM_CCER_CC2P;
  case PWM_OUTPUT_ACTIVE_HIGH:
    ccer |= TIM_CCER_CC2E;
  default:
    ;
  }
  switch (pwmp->config->channels[2].mode & PWM_OUTPUT_MASK) {
  case PWM_OUTPUT_ACTIVE_LOW:
    ccer |= TIM_CCER_CC3P;
  case PWM_OUTPUT_ACTIVE_HIGH:
    ccer |= TIM_CCER_CC3E;
  default:
    ;
  }
  switch (pwmp->config->channels[3].mode & PWM_OUTPUT_MASK) {
  case PWM_OUTPUT_ACTIVE_LOW:
    ccer |= TIM_CCER_CC4P;
  case PWM_OUTPUT_ACTIVE_HIGH:
    ccer |= TIM_CCER_CC4E;
  default:
    ;
  }
#if STM32_PWM_USE_ADVANCED
#if STM32_PWM_USE_TIM1 && !STM32_PWM_USE_TIM8
  if (&PWMD1 == pwmp) {
#endif
#if !STM32_PWM_USE_TIM1 && STM32_PWM_USE_TIM8
  if (&PWMD8 == pwmp) {
#endif
#if STM32_PWM_USE_TIM1 && STM32_PWM_USE_TIM8
  if ((&PWMD1 == pwmp) || (&PWMD8 == pwmp)) {
#endif
    switch (pwmp->config->channels[0].mode & PWM_COMPLEMENTARY_OUTPUT_MASK) {
    case PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW:
      ccer |= TIM_CCER_CC1NP;
    case PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH:
      ccer |= TIM_CCER_CC1NE;
    default:
      ;
    }
    switch (pwmp->config->channels[1].mode & PWM_COMPLEMENTARY_OUTPUT_MASK) {
    case PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW:
      ccer |= TIM_CCER_CC2NP;
    case PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH:
      ccer |= TIM_CCER_CC2NE;
    default:
      ;
    }
    switch (pwmp->config->channels[2].mode & PWM_COMPLEMENTARY_OUTPUT_MASK) {
    case PWM_COMPLEMENTARY_OUTPUT_ACTIVE_LOW:
      ccer |= TIM_CCER_CC3NP;
    case PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH:
      ccer |= TIM_CCER_CC3NE;
    default:
      ;
    }
  }
#endif /* STM32_PWM_USE_ADVANCED*/

  pwmp->tim->CCER = ccer;
  pwmp->tim->EGR  = TIM_EGR_UG;             /* Update event.                */
  pwmp->tim->DIER = pwmp->config->callback == NULL ? 0 : TIM_DIER_UIE;
  pwmp->tim->SR   = 0;                      /* Clear pending IRQs.          */
#if STM32_PWM_USE_TIM1 || STM32_PWM_USE_TIM8
#if STM32_PWM_USE_ADVANCED
  pwmp->tim->BDTR = pwmp->config->bdtr | TIM_BDTR_MOE;
#else
  pwmp->tim->BDTR = TIM_BDTR_MOE;
#endif
#endif
  /* Timer configured and started.*/
  pwmp->tim->CR1  = TIM_CR1_ARPE | TIM_CR1_URS | TIM_CR1_CEN;
}

/**
 * @brief   Configures and activates the PWM peripheral.
 * @note    Starting a driver that is already in the @p PWM_READY state
 *          disables all the active channels.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] config    pointer to a @p PWMConfig object
 *
 * @api
 */
void my_pwmStart(PWMDriver *pwmp, const PWMConfig *config) {

  chDbgCheck((pwmp != NULL) && (config != NULL), "pwmStart");

  chSysLock();
  chDbgAssert((pwmp->state == PWM_STOP) || (pwmp->state == PWM_READY),
              "pwmStart(), #1", "invalid state");
  pwmp->config = config;
  pwmp->period = config->period;
  my_pwm_lld_start(pwmp);
  pwmp->state = PWM_READY;
  chSysUnlock();
}

void pwm_start() {
	//pwmStart(&PWMD3, &pwmcfg);
	//pwmEnableChannel(&PWMD3, 1, INIT_PWM_PULSE_WIDTH_TICS);

	pwmStart(&PWMD4, &pwmcfg);
	pwmEnableChannel(&PWMD4, 2, INIT_PWM_PULSE_WIDTH_TICS);
	//pwmEnableChannel(&PWMD4, 3, INIT_PWM_PULSE_WIDTH_TICS);
	// pwmDisableChannel(&PWMD4, 3);
}

uint32_t pwm_get_PWM_freq_hz() {
    return pwm_freq_hz;
}

pwmcnt_t pwm_get_pulse_width() {
    return pwm_pulse_width_ticks;
}

/*! \brief Return period
 *
 * @return period in mS
 */
uint32_t pwm_get_period_ms() {
    uint32_t per_ms = 0;

    per_ms   = (uint32_t) (pwm_period_ticks/pwm_freq_hz * 1000) ;

    return per_ms;
}

void pwm_set_pulse_width_ticks(uint32_t width_ticks) {
    //pwmDisableChannel(&PWMD4, 3);

    pwmEnableChannel(&PWMD4, 3, width_ticks);

    pwm_pulse_width_ticks   = width_ticks;
}

void pwm_set_period(uint32_t per_ticks)  {
    pwmChangePeriod(&PWMD4, per_ticks);

    pwm_period_ticks = per_ticks;
}

pwmcnt_t pwm_us_to_ticks(uint32_t us) {

    pwmcnt_t tickval           = 0;

    double   ticks_per_us_quot = 0;
    uint32_t ticks_per_us      = 0;

    ticks_per_us_quot          = pwm_freq_hz/1e6;
    ticks_per_us               = (pwmcnt_t) ticks_per_us_quot;

    tickval                    = (pwmcnt_t) (us * ticks_per_us);

    return tickval;
}

//! @}
