#include "audio_synthesis.h"

//sample rate = 44100

enum Note{
NOTE_C4,
NOTE_D4,
NOTE_E4,
NOTE_F4,
NOTE_G4,
NOTE_A4,
NOTE_B4,
};

static const float NOTE_FREQUENCY[NOTE_COUNT] = {261.63f, 293.66f, 329.63f, 349.23f,
    392.00f, 440.00f, 493.88f};

// Task 2: Synthesize audio and output to I2S
void audio_synthesis_task(void *pvParameters)
{
   i2s_chan_handle_t tx_handle;
   i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
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

   /* Start TX channel*/
   i2s_channel_enable(tx_handle);

   // clear the queue
   while(1){
      int note_index;
      
      while (xQueueReceive(note_queue, &note_index, 0) == pdPASS){
         // first, lets process the input notes pressed into another array full of Note objects
         // this inner loop is for PRESSED NOTES.
         
         // note_index= the number when pressed, -1 to 6
         // each iteration is for one entry of the queue only, determined by i

         if (note_index < NOTE_COUNT && note_index >= 0){
            output_notes[note_index].frequency = NOTE_FREQUENCY[note_index]; 
            output_notes[note_index].is_active = true; 
            output_notes[note_index].phase_increment = 2 * M_PI * output_notes[note_index].frequency / SAMPLE_RATE; // phase function;
            output_notes[note_index].phase = 0;
            output_notes[note_index].amplitude = 4; 
            output_notes[note_index].time_since_press = 0; 
            output_notes[note_index].start_time = xTaskGetTickCount();
         }
      }

      // all notes
      for (int j = 0; j < NOTE_COUNT; j++){

         uint32_t current_time = xTaskGetTickCount();

         if (output_notes[j].is_active == true){
            output_notes[j].time_since_press = current_time - output_notes[j].start_time; 

            output_notes[j].amplitude = exp(-2.0 * output_notes[j].time_since_press / 1000.0);

            if (output_notes[j].amplitude < 0.001){
               output_notes[j].is_active = false;
            }
         }
      }

         
      // go through 7 notes, output_notes

      //lets say this task goes every 0.005s, then i need the sample that is put out to last 0.005s. 
      //if its being sampled at 44100 hz, we need 0.005/(1/44100) = 220 samples per loop
      int16_t waveform[220] = {0};
      // a waveform is just sample0 sample1 sample2 sample3 ... sample219
      // each sample is composed of the sin of all the notes active
      for (int sample = 0; sample < 220; sample++){
         float mixed_sample = 0.0;
         for (int note = 0; note < NOTE_COUNT; note++){
            if (output_notes[note].is_active) {
                  mixed_sample += output_notes[note].amplitude * sin(output_notes[note].phase);
                  output_notes[note].phase += output_notes[note].phase_increment;
            }
         }
         waveform[sample] = (int16_t)(mixed_sample * 32767);
      }
      
      i2s_channel_write(tx_handle, waveform, 440, NULL, portMAX_DELAY);
   }
}
   