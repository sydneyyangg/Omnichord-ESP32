#ifndef SOFTPOT_SENSOR_H
#define SOFTPOT_SENSOR_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_adc/adc_oneshot.h"

// Potentiometer is connected to GPIO 34 (ADC1_CH6)
#define POT_CHANNEL ADC_CHANNEL_6  // GPIO 34

void softpot_read_task(void *pvParameters);

#endif