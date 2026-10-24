#include <stdio.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led.h"

static const char *TAG = "ESP32_RTM";

void app_main(void)
{
    esp_err_t ret = led_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LED initialization failed: %s", esp_err_to_name(ret));
    }

    while (1) {
        ret = led_toggle();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to toggle LED: %s", esp_err_to_name(ret));
            break;
        }
        ESP_LOGI(TAG, "LED state toggled");
        vTaskDelay(pdMS_TO_TICKS(1000));
        
    }

}
