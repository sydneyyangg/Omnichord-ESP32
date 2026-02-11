#ifndef AUDIO_SYNTHESIS_H
#define AUDIO_SYNTHESIS_H

#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/i2s_std.h"

void audio_synthesis_task(void *pvParameters);

#endif
