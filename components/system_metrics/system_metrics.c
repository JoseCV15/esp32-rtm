#include <stdio.h>

#include "system_metrics.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "led.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DEFAULT_HEAP_THRESHOLD_BYTES  (20 * 1024) // 20KB

static const char *TAG = "SYS_MONITOR";
static size_t heap_threshold_bytes = DEFAULT_HEAP_THRESHOLD_BYTES;
static portMUX_TYPE monitor_mux = portMUX_INITIALIZER_UNLOCKED;
static bool monitor_running = false;
static uint32_t monitor_interval_ms = 3000;

static TaskHandle_t monitor_task_handle = NULL;

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

static void system_metrics_monitor_task(void *pvParameters)
{
    (void)pvParameters;

    TickType_t last_wake = xTaskGetTickCount();
    bool low_memory_alert_active = false;

    ESP_LOGI(TAG, "Real-Time monitoring started");

    while (1) {
        bool running;
        uint32_t interval_ms;
        // Read monitor configuration safely
        portENTER_CRITICAL(&monitor_mux);
        running = monitor_running;
        interval_ms = monitor_interval_ms;
        portEXIT_CRITICAL(&monitor_mux);

        if (running) {
            system_metrics_t metrics;
            esp_err_t err = system_metrics_get(&metrics);

            if (err == ESP_OK) {

                size_t threshold = system_metrics_get_heap_threshold();
                system_metrics_log();
                // Check free memory
                if (metrics.free_heap_bytes < threshold) {

                    if (!low_memory_alert_active) {

                        ESP_LOGW(TAG,
                                 "LOW MEMORY ALERT! Free heap: %u bytes. Threshold: %u bytes",
                                 (unsigned int)metrics.free_heap_bytes,
                                 (unsigned int)threshold);

                        low_memory_alert_active = true;
                    }

                } else {

                    if (low_memory_alert_active) {

                        ESP_LOGI(TAG,
                                 "Memory recovered. Free heap: %u bytes",
                                 (unsigned int)metrics.free_heap_bytes);

                        low_memory_alert_active = false;
                    }
                }

            } else {

                ESP_LOGE(TAG,
                         "Failed to collect system metrics: %s",
                         esp_err_to_name(err));
            }
        }

        // Wait using the current configured interval
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(interval_ms));
    }
}

esp_err_t system_metrics_start_monitor(uint32_t interval_ms)
{
    if (interval_ms != 0) {
        esp_err_t err = system_metrics_set_interval(interval_ms);
        if (err != ESP_OK) {
            return err;
        }
    }

    portENTER_CRITICAL(&monitor_mux);
    monitor_running = true;
    portEXIT_CRITICAL(&monitor_mux);

    if (monitor_task_handle == NULL) {
        BaseType_t result = xTaskCreate(
            system_metrics_monitor_task,
            "sys_monitor",
            4096,
            NULL,
            4,
            &monitor_task_handle
        );

        if (result != pdPASS) {
            portENTER_CRITICAL(&monitor_mux);
            monitor_running = false;
            portEXIT_CRITICAL(&monitor_mux);
            monitor_task_handle = NULL;
            return ESP_ERR_NO_MEM;
        } 
    }

    return ESP_OK;
}


esp_err_t system_metrics_stop_monitor(void)
{
    portENTER_CRITICAL(&monitor_mux);
    monitor_running = false;
    portEXIT_CRITICAL(&monitor_mux);

    ESP_LOGI(TAG, "System monitor stopped");
    return ESP_OK;
}

esp_err_t system_metrics_set_interval(uint32_t interval_ms)
{
    if (interval_ms < 100 || interval_ms > 60000) {
        ESP_LOGE(TAG, "Invalid monitor interval: %lu ms", (unsigned long)interval_ms);
        return ESP_ERR_INVALID_ARG;
    }
    portENTER_CRITICAL(&monitor_mux);
    monitor_interval_ms = interval_ms;
    portEXIT_CRITICAL(&monitor_mux);

    ESP_LOGI(TAG, "Monitor interval set to %lu ms", (unsigned int)interval_ms);
    return ESP_OK;

}
esp_err_t system_metrics_get_monitor_status(bool *running, uint32_t *interval_ms)
{
    if (running == NULL || interval_ms == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    portENTER_CRITICAL(&monitor_mux);
    *running = monitor_running;
    *interval_ms = monitor_interval_ms;
    portEXIT_CRITICAL(&monitor_mux);

    return ESP_OK;
}


esp_err_t system_metrics_set_heap_threshold(size_t threshold_bytes)
{
    if (threshold_bytes == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    portENTER_CRITICAL(&monitor_mux);
    heap_threshold_bytes = threshold_bytes;
    portEXIT_CRITICAL(&monitor_mux);

    ESP_LOGI(TAG, "Heap alert threshold set to %u bytes", (unsigned int)threshold_bytes);
    return ESP_OK;
}

size_t system_metrics_get_heap_threshold(void)
{
    size_t threshold;

    portENTER_CRITICAL(&monitor_mux);
    threshold = heap_threshold_bytes;
    portEXIT_CRITICAL(&monitor_mux);

    return threshold;
}