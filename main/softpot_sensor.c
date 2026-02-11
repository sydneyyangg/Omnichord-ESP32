#include "softpot_sensor.h"

// Potentiometer is connected to GPIO 34 (ADC1_CH6)
#define POT_CHANNEL ADC_CHANNEL_6  // GPIO 34

void softpot_read_task(void *pVParameters)
{
    // Configure ADC
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config, &adc1_handle);

    // Configure channel
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,      // 12-bit resolution (0-4095)
        .atten = ADC_ATTEN_DB_12,         // Full range 0-3.3V
    };
    adc_oneshot_config_channel(adc1_handle, POT_CHANNEL, &config);
    
    // Wait 1 second
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    int potValue = 0;
    
    while(1) {
        // Read potentiometer value
        adc_oneshot_read(adc1_handle, POT_CHANNEL, &potValue);
        printf("%d\n", potValue);
        
        // 100ms delay
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}