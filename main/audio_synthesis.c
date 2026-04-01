#include "audio_synthesis.h"

i2s_chan_handle_t tx_handle;

// button states
bool last_state[NUM_BUTTONS]   = {false, false, false};
bool stable_state[NUM_BUTTONS] = {false, false, false};
TickType_t last_change[NUM_BUTTONS] = {0, 0, 0};

void configure_i2s(){
   i2s_chan_config_t chan_cfg = {
    .id = I2S_NUM_AUTO,
    .role = I2S_ROLE_MASTER,
    .dma_desc_num = 16,      // Increase from default (usually 6)
    .dma_frame_num = 480,   // Increase from default
    .auto_clear = true,
   };

   i2s_new_channel(&chan_cfg, &tx_handle, NULL);
   i2s_std_config_t std_cfg = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
      .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
      .gpio_cfg = {
         .mclk = I2S_GPIO_UNUSED,
         .bclk = GPIO_NUM_4,
         .ws = GPIO_NUM_5,
         .dout = GPIO_NUM_18,
         .din = I2S_GPIO_UNUSED,
         .invert_flags = {
            .mclk_inv = false,
            .bclk_inv = false,
            .ws_inv = false,
         },
      },
   };
   
   i2s_channel_init_std_mode(tx_handle, &std_cfg);
   i2s_channel_enable(tx_handle);

   for (int i = 0; i < NUM_BUTTONS; i++) {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << BUTTON_PINS[i]),
            .mode         = GPIO_MODE_INPUT,
            .pull_up_en   = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_ENABLE,
            .intr_type    = GPIO_INTR_DISABLE,
        };
        gpio_config(&io_conf);
    }
}
void audio_synthesis_task(void *pvParameters){
    esp_task_wdt_add(NULL);
    
    static float phase = 0;
    static float freq = 466.16;
    static float phase_inc = 2 * M_PI * 466.16 / 44100.0;
    static int current_chord = 0;
    static TickType_t press_start[NUM_BUTTONS] = {0};  // Track when button was pressed
    
    while(1){
        esp_task_wdt_reset();
        TickType_t now = xTaskGetTickCount();
        
        for (int i = 0; i < NUM_BUTTONS; i++) {
            bool current = gpio_get_level(BUTTON_PINS[i]);
            
            // Detect state change
            if (current != last_state[i]) {
                last_change[i] = now;
                last_state[i] = current;
            }
            
            // debounce
            if ((now - last_change[i]) >= pdMS_TO_TICKS(DEBOUNCE_MS)) {
                if (current != stable_state[i]) {
                    stable_state[i] = current;
                    
                    // press
                    if (stable_state[i] == true) {
                        press_start[i] = now;  // Record when pressed
                    }
                    // release
                    else {
                        TickType_t press_duration = now - press_start[i];
                        
                        // long press Bb major, short press G minor
                        if (i == 0) {
                            if (press_duration >= pdMS_TO_TICKS(500)) {
                                current_chord = 0;
                            } else {
                                current_chord = 1;
                            }
                        }
                        // f major
                        else if (i == 1) {
                            current_chord = 2;
                        }
                        //c minor
                        else if (i == 2) {
                            current_chord = 3;
                        }
                    }
                }
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
        
        // Check queue for note changes
        int note_index;
        static int last_note = -1;
        if (xQueueReceive(note_queue, &note_index, 0) == pdTRUE) {
            if (note_index != last_note && note_index >= 0 && note_index < NOTE_COUNT) {
                freq = CHORD_FREQUENCIES[current_chord][note_index];
                phase_inc = 2 * M_PI * freq / 44100.0;
                last_note = note_index;
            }
        }
        
        // Generate waveform
        int16_t waveform[512];
        for (int i = 0; i < 512; i++) {
            waveform[i] = (int16_t)(0.2 * sin(phase) * 32767);
            phase += phase_inc;
            if (phase > 2 * M_PI) phase -= 2 * M_PI;
        }
        
        size_t bytes_written;
        i2s_channel_write(tx_handle, waveform, 1024, &bytes_written, portMAX_DELAY);
    }
}