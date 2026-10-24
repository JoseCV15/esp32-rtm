#include <stdio.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led.h"
#include "system_metrics.h"

static const char *TAG = "ESP32_RTM";

void app_main(void)
{

    while (1) {
        

        system_metrics_log();
        vTaskDelay(pdMS_TO_TICKS(5000));
        
    }

}
