#include <stdio.h>
#include "system_metrics.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "led.h"


static const char *TAG = "system_metrics";

esp_err_t system_metrics_get(system_metrics_t *self)
{
    if (!self) {
        return ESP_ERR_INVALID_ARG;
    }

    self->uptime_ms = (uint64_t)(esp_timer_get_time() / 1000);
    self->free_heap_bytes = esp_get_free_heap_size();
    self->minimum_free_heap_bytes = esp_get_minimum_free_heap_size();
    self->free_internal_heap_bytes = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);

    esp_err_t err = led_get_state(&self->led_state);
    if (err != ESP_OK) {
        return err;
    }
  
    return ESP_OK;
}

void system_metrics_log(void)
{
    system_metrics_t metrics;
    esp_err_t ret = system_metrics_get(&metrics);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to collect system metrics");
        return;
    }

    ESP_LOGI(TAG, "----- System Metrics -------");
    ESP_LOGI(TAG, "Uptime:              %llu ms", (unsigned long long)metrics.uptime_ms);
    ESP_LOGI(TAG, "Free heap:           %u bytes", (unsigned int)metrics.free_heap_bytes);
    ESP_LOGI(TAG, "Minimum free heap:   %u bytes", (unsigned int)metrics.minimum_free_heap_bytes);
    ESP_LOGI(TAG, "Free internal heap:  %u bytes", (unsigned int)metrics.free_internal_heap_bytes);
    ESP_LOGI(TAG, "LED state: %s", metrics.led_state ? "ON" : "OFF");

}