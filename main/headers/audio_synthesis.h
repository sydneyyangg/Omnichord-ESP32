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

#define NOTE_COUNT 8
#define NUM_CHORDS 4
#define SAMPLE_RATE 44100
#define DEBOUNCE_MS     50
#define NUM_BUTTONS     3

static const gpio_num_t BUTTON_PINS[NUM_BUTTONS] = {
    GPIO_NUM_13,
    GPIO_NUM_14,
    GPIO_NUM_23,
};

void audio_synthesis_task(void *pvParameters);
void configure_i2s();
extern i2s_chan_handle_t tx_handle;

// 2D array: [chord][note_index]
static const float CHORD_FREQUENCIES[NUM_CHORDS][NOTE_COUNT] = {
    // Chord 0: Bb Major (Bb4 to Bb5)
    {466.16, 523.25, 587.33, 622.25, 698.46, 783.99, 880.00, 932.33},
    
    // Chord 1: G Minor (G4 to G5)
    {392.00, 440.00, 466.16, 523.25, 587.33, 622.25, 698.46, 783.99},
    
    // Chord 2: F Major (F4 to F5)
    {349.23, 392.00, 440.00, 466.16, 523.25, 587.33, 659.25, 698.46},
    
    // Chord 3: C Minor (C4 to C5)
    {523.25, 587.33, 622.25, 698.46, 783.99, 830.61, 932.33, 1046.50}
};

extern const int16_t WAVETABLE[256];
#endif
