#include "audio_synthesis.h"

// Task 2: Synthesize audio and output to I2S
void audio_synthesis_task(void *pvParameters)
{
   int notes_pressed[64]; // holds the ints of notes pressed, -1 to 6.
   // need to get an rtos queue from softpot sensor
   xQueueReceive(note_queue, &notes_pressed, 0);

   // make a struct that holds freq, isactive, phase, volume/amp, time
   typedef struct {
      int frequency;
      bool is_active;
      int phase;
      int amplitude; // vol
      int time_since_press;
   } Note;

   // calculate phase

   // process the queue into an audio wave w phase
   
   // push it into the dac + amp module with i2s
   // profit
}