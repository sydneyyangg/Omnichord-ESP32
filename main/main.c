#include "main.h"

void app_main(void)
{    
    // setup 
    configure_softpot_task();
    configure_i2s();
    
    // Create tasks
    xTaskCreatePinnedToCore(softpot_read_task, "softpot", 4096, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(audio_synthesis_task, "audio", 16384, NULL, 20, NULL, 1);  // Higher priority

}