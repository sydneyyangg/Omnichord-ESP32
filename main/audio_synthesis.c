#include "audio_synthesis.h"

// Task 2: Synthesize audio and output to I2S
void audio_synthesis_task(void *pvParameters)
{
   
   // clear the queue
   while(1){
      int note_index;
      while (xQueueReceive(note_queue, &note_index, 0) == pdPASS){
         // first, lets process the input notes pressed into another array full of Note objects
         // this inner loop is for PRESSED NOTES.
         
         // note_index= the number when pressed, -1 to 6
         // each iteration is for one entry of the queue only, determined by i
         

         if (note_index != -1){
            output_notes[note_index].frequency = 0; // is this enum time? based on a defined chord, then on number, out a frequency
            output_notes[note_index].is_active = true; // true
            output_notes[note_index].phase_increment = 2 * M_PI * output_notes[note_index].frequency / 44100; // phase function;
            output_notes[note_index].phase = 0;
            output_notes[note_index].amplitude = 4; //4 * exp(-6 * time);  exp decay for sound decrease
            output_notes[note_index].time_since_press = 0; // current time - start time
         }
         

      }

      // unprocessed notes
      // change:
      // is active
      // phase
      // amplitude
      // time since press


      uint32_t start_time = xTaskGetTickCount(); // fix placement


      for (int j = 0; j < 7; j++){

         uint32_t current_time = xTaskGetTickCount();

         if (output_notes[j].is_active == true){
            output_notes[j].time_since_press = current_time - start_time; // ehhh kinda cooked
            output_notes[j].phase += output_notes[j].phase_increment;

            output_notes[j].amplitude = exp(-2.0 * output_notes[j].time_since_press);

            if (output_notes[j].amplitude < 0.001){
               output_notes[j].is_active = false;
            }

         }
      }

    }
   
   //c4 d4 e4 f4 g4 a4 b4 c5

   // process the queue into an audio wave w phase
   
   // go through 7 notes, output_notes
   // check is_active. if active, it makes sound, and make a sin wave of each

   // need sin of note 1, + sin of note 2, ...
   // sin of note x is also a buffer of 220 samples.
      // loop of 220:
         // each value is: amp * sin(phase)
         // where phase starts off as 0 but then increments with phase increment
   float waveform[220];
   // push waveform out into the dac + amp module with i2s
   // profit
   
   }
   
}