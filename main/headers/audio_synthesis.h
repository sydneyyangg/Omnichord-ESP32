#ifndef AUDIO_SYNTHESIS_H
#define AUDIO_SYNTHESIS_H

#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/i2s_std.h"
#include "softpot_sensor.h"
#include "esp_task_wdt.h"

#define NOTE_COUNT 7
#define SAMPLE_RATE 44100
void audio_synthesis_task(void *pvParameters);
void configure_i2s();
extern i2s_chan_handle_t tx_handle;


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
   
extern Note output_notes[7]; 

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

#endif
