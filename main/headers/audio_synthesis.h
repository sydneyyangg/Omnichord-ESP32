#ifndef AUDIO_SYNTHESIS_H
#define AUDIO_SYNTHESIS_H

#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/i2s_std.h"
#include "softpot_sensor.h"

#define NOTE_COUNT 7
void audio_synthesis_task(void *pvParameters);

// make a struct that holds freq, isactive, phase, volume/amp, time
   typedef struct {
      float frequency;
      bool is_active;
      float phase;
      float phase_increment;
      float amplitude; // vol
      float time_since_press;
      uint32_t start_time;
   } Note;
   
Note output_notes[7]; 

#endif
