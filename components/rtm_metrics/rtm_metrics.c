#include "rtm_metrics.h"

#include <inttypes.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_heap_caps.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

/* Configuration */
static const char *TAG = "RTM_Metrics";
#define RTM_METRICS_DEFAULT_PERIOD_MS        3000U
#define RTM_METRICS_TASK_STACK_SIZE           4096
#define RTM_METRICS_TASK_PRIORITY             2
#define RTM_METRICS_HEAP_CAPS                 MALLOC_CAP_8BIT
#define RTM_METRICS_INTERNAL_CAPS             (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
//#define RTM_METRICS_PSRAM_CAPS                (MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)

/* Internal state */
static TaskHandle_t s_monitor_task_handle = NULL;
// Protect shared state
static SemaphoreHandle_t s_metrics_mutex = NULL;
/**
 * @brief Semaphore to signal a task has finished
 */
static SemaphoreHandle_t s_task_stopped_sem = NULL;

static bool s_initialized = false;
static uint32_t s_period_ms = RTM_METRICS_DEFAULT_PERIOD_MS;

static uint8_t number_of_cpu_cores(void) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    return chip_info.cores;
}

// TODO: agregate in json format in the future
static void rtm_metrics_print_data(const rtm_metrics_t *metrics)
{
    ESP_LOGI(TAG, "----- RTM Metrics -------");
    ESP_LOGI(TAG, "Uptime: %" PRIu64 " ms", metrics->uptime);
    ESP_LOGI(TAG, "Reset reason: %d", (int)metrics->rst_reason);
    ESP_LOGI(TAG, "CPU cores: %d", (int)metrics->cpu_core_count);
    ESP_LOGI(TAG, "FreeRTOS tasks: %" PRIu32, metrics->task_count);

    ESP_LOGI(TAG, "Total heap: %zu bytes", metrics->total_heap);
    ESP_LOGI(TAG, "Free heap: %zu bytes", metrics->free_heap);
    ESP_LOGI(TAG, "Minimum free heap: %zu bytes", metrics->minimum_free_heap);
    ESP_LOGI(TAG, "Largest free block heap: %zu bytes", metrics->largest_free_block);

    ESP_LOGI(TAG, "Total Iram: %zu bytes", metrics->total_iram_heap);
    ESP_LOGI(TAG, "Free Iram: %zu bytes", metrics->free_iram_heap);
    ESP_LOGI(TAG, "Minimum free Iram: %zu bytes", metrics->min_free_iram_heap);
    ESP_LOGI(TAG, "Largest free block Iram: %zu bytes", metrics->largest_iram_free_block);
}

static uint32_t get_monitor_period(void)
{
    uint32_t period_ms = RTM_METRICS_DEFAULT_PERIOD_MS;

    if (s_metrics_mutex == NULL) {
        return period_ms;
    }
    if (xSemaphoreTake(s_metrics_mutex, portMAX_DELAY) == pdTRUE) {
        period_ms = s_period_ms;
        xSemaphoreGive(s_metrics_mutex);
    }
    return period_ms;
}


static void monitor_task(void *pvParameters)
{
    (void)pvParameters;

    ESP_LOGI(TAG, "Metrics monitor task started");

    while (true) {
        rtm_metrics_print();
        uint32_t period_ms = get_monitor_period();
        TickType_t delay_ticks = pdMS_TO_TICKS(period_ms);

        if (ulTaskNotifyTake(pdTRUE, delay_ticks) > 0) {
            break;
        }
    }

    ESP_LOGI(TAG, "Metrics monitor stopping");

    // Clear the task before notifying the caller
    if (xSemaphoreTake(s_metrics_mutex, portMAX_DELAY) == pdTRUE) {
        s_monitor_task_handle = NULL;
        xSemaphoreGive(s_metrics_mutex);
    }

    xSemaphoreGive(s_task_stopped_sem);
    vTaskDelete(NULL);
}

/* Public API */

esp_err_t rtm_metrics_init(void)
{
    if (s_initialized) {
        return ESP_OK;
    }

    s_metrics_mutex = xSemaphoreCreateMutex();
    if (s_metrics_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create the mutex");
        return ESP_ERR_NO_MEM;
    }

    s_task_stopped_sem = xSemaphoreCreateBinary();
    if (s_task_stopped_sem == NULL) {
        ESP_LOGE(TAG, "Failed to create the semaphore");
        vSemaphoreDelete(s_metrics_mutex);
        s_metrics_mutex = NULL;
        return ESP_ERR_NO_MEM;
    }

    s_period_ms = RTM_METRICS_DEFAULT_PERIOD_MS;
    s_monitor_task_handle = NULL;
    s_initialized = true;

    ESP_LOGI(TAG, "RTM Metrics initialized");

    return ESP_OK;
}


esp_err_t rtm_metrics_get(rtm_metrics_t *metrics)
{
    if (!metrics) {
        return ESP_ERR_INVALID_ARG;
    }

    metrics->uptime =  (uint64_t)(esp_timer_get_time() / 1000ULL);
    metrics->rst_reason = esp_reset_reason();
    metrics->task_count = (uint32_t)uxTaskGetNumberOfTasks();
    metrics->cpu_core_count = (uint32_t)number_of_cpu_cores();

    metrics->total_heap = heap_caps_get_total_size(RTM_METRICS_HEAP_CAPS);
    metrics->free_heap = heap_caps_get_free_size(RTM_METRICS_HEAP_CAPS);
    metrics->minimum_free_heap = heap_caps_get_minimum_free_size(RTM_METRICS_HEAP_CAPS);
    metrics->largest_free_block = heap_caps_get_largest_free_block(RTM_METRICS_HEAP_CAPS);

    metrics->total_iram_heap = heap_caps_get_total_size(RTM_METRICS_INTERNAL_CAPS);
    metrics->free_iram_heap = heap_caps_get_free_size(RTM_METRICS_INTERNAL_CAPS);
    metrics->min_free_iram_heap = heap_caps_get_minimum_free_size(RTM_METRICS_INTERNAL_CAPS);
    metrics->largest_iram_free_block = heap_caps_get_largest_free_block(RTM_METRICS_INTERNAL_CAPS);

    return ESP_OK;
}

void rtm_metrics_print(void)
{
    rtm_metrics_t metrics = {0};
    esp_err_t err = rtm_metrics_get(&metrics);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to collect metrics");
    }
    else {
        rtm_metrics_print_data(&metrics);
    }
}

esp_err_t rtm_metrics_start(uint32_t period_ms)
{
    BaseType_t res = pdFAIL;

    if (period_ms == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!s_initialized || !s_metrics_mutex || !s_task_stopped_sem) {
        return ESP_ERR_INVALID_STATE;
    }

    if (xSemaphoreTake(s_metrics_mutex, portMAX_DELAY) != pdTRUE) {
        return ESP_FAIL;
    }

    if (s_monitor_task_handle != NULL) {
        xSemaphoreGive(s_metrics_mutex);
        return ESP_ERR_INVALID_STATE;
    }

    s_period_ms = period_ms;
    // Ensure the semaphore is empty
    (void)xSemaphoreTake(s_task_stopped_sem, 0);

    res = xTaskCreate(
        monitor_task,
        "rtm_metrics",
        RTM_METRICS_TASK_STACK_SIZE,
        NULL,
        RTM_METRICS_TASK_PRIORITY,
        &s_monitor_task_handle
    );

    if (res != pdPASS) {
        s_monitor_task_handle = NULL;
        ESP_LOGE(TAG, "Failed to create metrics monitor task");
        return ESP_ERR_NO_MEM;
    }

    xSemaphoreGive(s_metrics_mutex);

    ESP_LOGI(TAG, "Metrics monitoring enabled: %" PRIu32 " ms", period_ms);
    return ESP_OK;

}

esp_err_t rtm_metrics_stop(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (xSemaphoreTake(s_metrics_mutex, portMAX_DELAY) != pdTRUE) {
        return ESP_FAIL;
    }

    if (s_monitor_task_handle == NULL) {
        xSemaphoreGive(s_metrics_mutex);
        return ESP_ERR_INVALID_STATE;
    }

    xTaskNotifyGive(s_monitor_task_handle);
    xSemaphoreGive(s_metrics_mutex);

    if (xSemaphoreTake(s_task_stopped_sem, portMAX_DELAY) != pdTRUE) {
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Metrics monitoring stopped");
    return ESP_OK;

}


esp_err_t rtm_metrics_set_period(uint32_t period_ms)
{
    if (!s_initialized) return ESP_ERR_INVALID_STATE;
    if (period_ms == 0) return ESP_ERR_INVALID_ARG;
    
    if (xSemaphoreTake(s_metrics_mutex, portMAX_DELAY) == pdTRUE) {
        s_period_ms = period_ms;
        xSemaphoreGive(s_metrics_mutex);
    }
    ESP_LOGI(TAG, "Metrics period changed to %" PRIu32 " ms", period_ms);
    return ESP_OK;
}

esp_err_t rtm_metrics_get_status(bool *running, uint32_t *period_ms)
{
    if (!s_initialized) return ESP_ERR_INVALID_STATE;
    if (!running || !period_ms) return ESP_ERR_INVALID_ARG;

    if (xSemaphoreTake(s_metrics_mutex, portMAX_DELAY) == pdTRUE) {
        *running = (s_monitor_task_handle != NULL);
        *period_ms = s_period_ms;
        xSemaphoreGive(s_metrics_mutex);
    }
    return ESP_OK;
}
