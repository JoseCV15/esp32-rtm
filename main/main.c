#include <stdio.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led.h"
#include "diagnostic_shell.h"

static const char *TAG = "ESP32_RTM";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting ESP32 Real-Time Monitor");

    esp_err_t err = led_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LED initialization failed: %s", esp_err_to_name(err));
        return;
    }

    err = diagnostic_shell_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Diagnostic shell startup failed: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "ESP32 Real-Time Monitor ready");

    
}
