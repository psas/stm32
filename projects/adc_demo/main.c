


#define ADC_GRP1_NUM_CHANNELS 1


/*
 * ADC conversion group 1.
 * Mode:       
 * Channels:    IN10.
 */
static const ADCConversionGroup adcgrpcfg1 = {
    .circular = FALSE,                                           // circular
    .num_channels = ADC_GRP1_NUM_CHANNELS,
    .end_cb = NULL,                                            // end cb
    .error_cb = adcerrorcallback,                                // error cb
    .cr1 = (ADC_CR1_DISCEN | ADC_CR1_EOCIE),                // CR1
    .cr2 = ADC_CR2_SWSTART,                                 // CR2  ??
    .smpr1 = ADC_SMPR1_SMP_AN11(ADC_SAMPLE_480),
    .smpr2 = 0,                                               // SMPR2
    .sqr1 = ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS),
    .sqr2 = 0,                                               // SQR2
    .sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN10)
};

/*
 * ADC conversion group.
 * Mode:        Linear buffer, 8 samples of 1 channel, SW triggered.
 * Channels:    IN11.
 */
static const ADCConversionGroup adcgrpcfg2 = {
    FALSE,
    ADC_GRP1_NUM_CHANNELS,
    NULL,
    adcerrorcallback,
    0,                        /* CR1 */
    ADC_CR2_SWSTART,          /* CR2 */
    ADC_SMPR1_SMP_AN11(ADC_SAMPLE_480),
    0,                        /* SMPR2 */
    ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS),
    0,                        /* SQR2 */
    ADC_SQR3_SQ1_N(ADC_CHANNEL_IN11)
};

