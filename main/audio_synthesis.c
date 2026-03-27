#include "audio_synthesis.h"

i2s_chan_handle_t tx_handle;
Note output_notes[NOTE_COUNT];

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
}

void audio_synthesis_task(void *pvParameters){
    esp_task_wdt_add(NULL);
    
    static float phase = 0;
    static float freq = 261.63;  // Middle C
    static float phase_inc = 2 * M_PI * 261.63 / 44100.0;
    
    while(1){
        esp_task_wdt_reset();
        
        // Simple: just check queue for note changes
        int note_index;
        static int last_note = -1;
        if (xQueueReceive(note_queue, &note_index, 0) == pdTRUE) {
            if (note_index != last_note && note_index >= 0 && note_index < NOTE_COUNT) {
                freq = NOTE_FREQUENCY[note_index];
                phase_inc = 2 * M_PI * freq / 44100.0;
                //phase = 0;  // Reset phase on new note
                last_note = note_index;
            }
        }
        
        // Generate simple continuous tone
        int16_t waveform[512];
        for (int i = 0; i < 512; i++) {
            waveform[i] = (int16_t)(0.4 * sin(phase) * 32767);
            phase += phase_inc;
            if (phase > 2 * M_PI) phase -= 2 * M_PI;
        }
        
        size_t bytes_written;
        i2s_channel_write(tx_handle, waveform, 1024, &bytes_written, portMAX_DELAY);
    }
}