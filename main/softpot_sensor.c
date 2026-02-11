#include "softpot_sensor.h"

QueueHandle_t note_queue;

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
    int i = 0;
    int note_index = 0;
    int last_note = 0;

     // Create a queue capable of containing 10 uint32_t values.
    note_queue = xQueueCreate( 64, sizeof( int ) );

    while(1) {
        // Read potentiometer value
        adc_oneshot_read(adc1_handle, POT_CHANNEL, &potValue);

        // if (i % 20 == 0)
        //     printf("%d\n", potValue);
        
            // Map ADC value to note (adjust ranges for your softpot)
        if (potValue < 10) {
            note_index = -1;  // No touch
        } else if (potValue < 285) {
            note_index = 0;  // C
        } else if (potValue < 470) {
            note_index = 1;  // D
        } else if (potValue < 655) {
            note_index = 2;  // E
        } else if (potValue < 840) {
            note_index = 3;  // F
        } else if (potValue < 1025) {
            note_index = 4;  // G
        } else if (potValue < 2100) {
            note_index = 5;  // A
        } else {
            note_index = 6;  // B
        }

        // Only send if note changed
        if (note_index != last_note) {
            xQueueSend(note_queue, &note_index, 0);
            last_note = note_index;
            printf("Note: %d\n", note_index);
        }

        //i++;
        // 100ms delay
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}