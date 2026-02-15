#include "main.h"

void app_main(void)
{    
    // setup 
    configure_softpot_task();
    // Create tasks
    xTaskCreate(softpot_read_task, "softpot", 2048, NULL, 5, NULL);
    //xTaskCreate(audio_synthesis_task, "audio", 4096, NULL, 10, NULL);  // Higher priority
    
}