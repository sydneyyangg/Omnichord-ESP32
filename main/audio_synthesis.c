#include "audio_synthesis.h"

i2s_chan_handle_t tx_handle;
Note output_notes[7];

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

// Task 2: Synthesize audio and output to I2S
// void audio_synthesis_task(void *pvParameters){

//    // // clear the queue
//     while(1){
//       int note_index;
      
//       while (xQueueReceive(note_queue, &note_index, 0) == pdPASS){
//          // first, lets process the input notes pressed into another array full of Note objects
//          // this inner loop is for PRESSED NOTES.
         
//          // note_index= the number when pressed, -1 to 6
//          // each iteration is for one entry of the queue only, determined by i

//          if (note_index < NOTE_COUNT && note_index >= 0){
//             output_notes[note_index].frequency = NOTE_FREQUENCY[note_index]; 
//             output_notes[note_index].is_active = true; 
//             output_notes[note_index].phase_increment = 2 * M_PI * output_notes[note_index].frequency / SAMPLE_RATE; // phase function;
//             output_notes[note_index].phase = 0;
//             output_notes[note_index].amplitude = 0.05; 
//             output_notes[note_index].time_since_press = 0; 
//             output_notes[note_index].start_time = xTaskGetTickCount();
//          }
//       }

//       uint32_t current_time = xTaskGetTickCount();

//       // all notes
//       for (int j = 0; j < NOTE_COUNT; j++){

//          if (output_notes[j].is_active == true){
//             output_notes[j].time_since_press = current_time - output_notes[j].start_time; 

//             output_notes[j].amplitude = 0.1*exp(-3.0 * (output_notes[j].time_since_press / 1000.0));

//             if (output_notes[j].amplitude < 0.001){
//                output_notes[j].is_active = false;
//             }
//          }
//       }

         
//       // go through 7 notes, output_notes

//        int16_t waveform[2048] = {0};
//       // a waveform is just sample0 sample1 sample2 sample3 ... sample219
//       // each sample is composed of the sin of all the notes active
//       for (int sample = 0; sample < 2048; sample++){
//          float mixed_sample = 0.0;
//          for (int note = 0; note < NOTE_COUNT; note++){
//             if (output_notes[note].is_active) {
//                   mixed_sample += output_notes[note].amplitude * sin(output_notes[note].phase);
//                   output_notes[note].phase += output_notes[note].phase_increment;
//                   if (output_notes[note].phase > 2.0 * M_PI) {
//                      output_notes[note].phase -= 2.0 * M_PI;
//                   }
//             }
//          }
//          waveform[sample] = (int16_t)(mixed_sample * 32767);
//       }
   
//       // // TEST:
//       // for (int sample = 0; sample < 2048; sample++){
//       //    static float phase = 0;
//       //    float simple_tone = 0.1 * sin(phase);
//       //    waveform[sample] = (int16_t)(simple_tone * 32767);
//       //    phase += 2 * M_PI * 440.0 / 44100.0;
//       //    if (phase > 2 * M_PI) phase -= 2 * M_PI;
//       // }

//       size_t bytes_written;
      
//       i2s_channel_write(tx_handle, waveform, 4096, &bytes_written, portMAX_DELAY);
//    }
// }

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
            if (note_index != last_note && note_index >= 0 && note_index < 7) {
                freq = NOTE_FREQUENCY[note_index];
                phase_inc = 2 * M_PI * freq / 44100.0;
                //phase = 0;  // Reset phase on new note
                last_note = note_index;
            }
        }
        
        // Generate simple continuous tone
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