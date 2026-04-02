//test this with the softpot sensor and see if it acc works lol, with new setup function

#include "softpot_sensor.h"

QueueHandle_t note_queue;
adc_oneshot_unit_handle_t adc1_handle;

void configure_softpot_task(){
// Configure ADC
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

     // Create a queue capable of containing 64 int values.
    note_queue = xQueueCreate(10, sizeof(int));
}

void softpot_read_task(void *pVParameters)
{
    int note_index = -1;
    int last_note = -1;
    
    while(1) {
        // Average 4 readings to reduce noise
        int sum = 0;
        for (int i = 0; i < 4; i++) {
            int reading;
            adc_oneshot_read(adc1_handle, POT_CHANNEL, &reading);
            sum += reading;
            vTaskDelay(2 / portTICK_PERIOD_MS);
        }
        int potValue = sum / 4;
        
        // Same threshold logic as before
        if (potValue < 50) {  // ← Bigger dead zone
            note_index = -1;
        } else if (potValue < 200) {  // ← Bigger range for note 0
            note_index = 0;
        } else if (potValue < 400) {
            note_index = 1;
        } else if (potValue < 600) {
            note_index = 2; //
        } else if (potValue < 750) {
            note_index = 3;
        } else if (potValue < 900) {
            note_index = 4;
        } else if (potValue < 1500) {
            note_index = 5; //
        } else if (potValue < 2100) {
            note_index = 6;
        } else {
            note_index = 7;
        }
        
        if (note_index != last_note) {  // Remove the "&& note_index >= 0"
            xQueueSendToBack(note_queue, &note_index, 0);
            last_note = note_index;
            printf("Softpot: %d (ADC: %d)\n", note_index, potValue);  // ADD THIS
        }
        
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}