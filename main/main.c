#include <stdio.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led.h"
#include "led_controller.h"
#include "diagnostic_shell.h"
#include "system_metrics.h"

static const char *TAG = "ESP32_RTM";

void app_main(void)
{
    ESP_LOGI(TAG, "Application started");

    // Initialize LED
    esp_err_t err = led_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LED: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "LED initialized");

    // Initialize LED controller
    err = led_controller_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LED controller: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "LED controller initialized");

    // Start diagnostic shell
    err = diagnostic_shell_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start Diagnostic Shell: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "Diagnostic Shell started");
    ESP_LOGI(TAG, "Application ready");
}
