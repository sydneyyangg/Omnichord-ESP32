#ifndef SOFTPOT_SENSOR_H
#define SOFTPOT_SENSOR_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"

void softpot_read_task(void *pvParameters);

#endif