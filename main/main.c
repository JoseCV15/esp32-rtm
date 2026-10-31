#include <stdio.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led.h"
#include "diagnostic_shell.h"

static const char *TAG = "ESP32_RTM";

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "led.h"


void app_main(void)
{
    ESP_LOGI(TAG, "Application started");

    esp_err_t err = led_init();

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LED initialization failed");
        return;
    }

    ESP_LOGI(TAG, "LED initialized");

    err = diagnostic_shell_start();
    if (err != ESP_OK) {
        ESP_LOGE("ESP32_RTM", "Diagnostic shell failed: %s",
                 esp_err_to_name(err));
        return;
    }
}
