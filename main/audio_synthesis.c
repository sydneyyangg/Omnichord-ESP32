#include "audio_synthesis.h"

 const int16_t WAVETABLE[256] = {
    0, 1921, 3829, 5706, 7540, 9317, 11023, 12647 ,
    14178, 15605, 16921, 18119, 19191, 20135, 20948, 21628 ,
    22177, 22595, 22886, 23055, 23107, 23049, 22890, 22636 ,
    22299, 21888, 21412, 20882, 20309, 19703, 19072, 18428 ,
    17778, 17132, 16496, 15878, 15282, 14714, 14179, 13678 ,
    13214, 12787, 12399, 12048, 11733, 11453, 11203, 10982 ,
    10786, 10610, 10452, 10307, 10171, 10040, 9911, 9781 ,
    9646, 9504, 9352, 9190, 9015, 8828, 8628, 8416 ,
    8191, 7956, 7713, 7462, 7206, 6947, 6688, 6431 ,
    6179, 5933, 5697, 5472, 5260, 5063, 4881, 4715 ,
    4567, 4436, 4322, 4224, 4141, 4073, 4018, 3973 ,
    3938, 3910, 3886, 3866, 3846, 3825, 3801, 3772 ,
    3735, 3691, 3638, 3574, 3500, 3414, 3318, 3211 ,
    3094, 2967, 2832, 2689, 2541, 2388, 2232, 2074 ,
    1915, 1758, 1604, 1453, 1306, 1166, 1031, 903 ,
    781, 666, 557, 454, 357, 263, 173, 86 ,
    0, -86, -173, -263, -357, -454, -557, -666 ,
    -781, -903, -1031, -1166, -1306, -1453, -1604, -1758 ,
    -1915, -2074, -2232, -2388, -2541, -2689, -2832, -2967 ,
    -3094, -3211, -3318, -3414, -3500, -3574, -3638, -3691 ,
    -3735, -3772, -3801, -3825, -3846, -3866, -3886, -3910 ,
    -3938, -3973, -4018, -4073, -4141, -4224, -4322, -4436 ,
    -4567, -4715, -4881, -5063, -5260, -5472, -5697, -5933 ,
    -6179, -6431, -6688, -6947, -7206, -7462, -7713, -7956 ,
    -8191, -8416, -8628, -8828, -9015, -9190, -9352, -9504 ,
    -9646, -9781, -9911, -10040, -10171, -10307, -10452, -10610 ,
    -10786, -10982, -11203, -11453, -11733, -12048, -12399, -12787 ,
    -13214, -13678, -14179, -14714, -15282, -15878, -16496, -17132 ,
    -17778, -18428, -19072, -19703, -20309, -20882, -21412, -21888 ,
    -22299, -22636, -22890, -23049, -23107, -23055, -22886, -22595 ,
    -22177, -21628, -20948, -20135, -19191, -18119, -16921, -15605 ,
    -14178, -12647, -11023, -9317, -7540, -5706, -3829, -1921 ,
};

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

    typedef struct {
        bool active;
        float phase;
        float speed;
        TickType_t start_tick;
        float amplitude;
    } synth_voice_t;

    #define MAX_VOICES 6
    #define VOICE_PEAK_AMPLITUDE 0.18f
    #define RELEASE_TIME_SEC 2.0f

    static int current_chord = 0;
    static TickType_t press_start[NUM_BUTTONS] = {0};  // Track when button was pressed
    static synth_voice_t voices[MAX_VOICES] = {0};
    
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
        
        // Check queue for note changes.
        int note_index;
        while (xQueueReceive(note_queue, &note_index, 0) == pdTRUE) {
            if (note_index >= 0 && note_index < NOTE_COUNT) {
                float freq = CHORD_FREQUENCIES[current_chord][note_index];

                // Find a free voice; if all are busy, steal the quietest one.
                int voice_idx = -1;
                float quietest_amp = 999.0f;
                for (int v = 0; v < MAX_VOICES; v++) {
                    if (!voices[v].active) {
                        voice_idx = v;
                        break;
                    }
                    if (voices[v].amplitude < quietest_amp) {
                        quietest_amp = voices[v].amplitude;
                        voice_idx = v;
                    }
                }

                voices[voice_idx].active = true;
                voices[voice_idx].phase = 0.0f;
                voices[voice_idx].speed = (freq * 256.0f) / (float)SAMPLE_RATE;
                voices[voice_idx].start_tick = now;
                voices[voice_idx].amplitude = VOICE_PEAK_AMPLITUDE;
            }
        }
   
        // Update each voice envelope.
        for (int v = 0; v < MAX_VOICES; v++) {
            if (voices[v].active) {
                TickType_t elapsed = now - voices[v].start_tick;
                float time_sec = (float)elapsed / (float)configTICK_RATE_HZ;

                if (time_sec >= RELEASE_TIME_SEC) {
                    voices[v].amplitude = 0.0f;
                    voices[v].active = false;
                } else {
                    voices[v].amplitude = VOICE_PEAK_AMPLITUDE * (1.0f - (time_sec / RELEASE_TIME_SEC));
                }
            }
        }

        // Generate and mix all active voices.
        int16_t waveform[512];
        for (int i = 0; i < 512; i++) {
            float mix = 0.0f;

            for (int v = 0; v < MAX_VOICES; v++) {
                if (!voices[v].active) {
                    continue;
                }

                int index = ((int)voices[v].phase) & 0xFF;
                mix += voices[v].amplitude * (float)WAVETABLE[index];

                voices[v].phase += voices[v].speed;
                if (voices[v].phase >= 256.0f) {
                    voices[v].phase -= 256.0f;
                }
            }

            if (mix > 32767.0f) {
                mix = 32767.0f;
            } else if (mix < -32768.0f) {
                mix = -32768.0f;
            }

            waveform[i] = (int16_t)mix;
        }
        
        size_t bytes_written;
        i2s_channel_write(tx_handle, waveform, 1024, &bytes_written, portMAX_DELAY);
    }
}