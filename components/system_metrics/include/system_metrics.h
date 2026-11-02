#pragma once

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    // Runtime health
    uint64_t uptime_ms;
    //float cpu_utilization_percent;
    uint32_t task_count;

    // Memory health
    size_t free_heap_bytes;
    size_t minimum_free_heap_bytes;
    size_t free_internal_heap_bytes;
    size_t largest_free_block_bytes;
    float heap_fragmentation_percent;

    // Reliability health
    esp_reset_reason_t rst_reason;

    // Hardware Health
    bool led_state;
}system_metrics_t;

/**
 * @brief Get current system health information
 */
esp_err_t system_metrics_get(system_metrics_t *self);

/**
 * @brief Log current system health information
 */
void system_metrics_log(void);

/**
 * @brief Start period system monitoring
 * @param interval_ms Monitoring interval in ms
 */
esp_err_t system_metrics_start_monitor(uint32_t interval_ms);

/**
 * @brief Stop system health monitoring
 */
esp_err_t system_metrics_stop_monitor(void);

/**
 * @brief Change monitoring interval
 */
esp_err_t system_metrics_set_interval(uint32_t interval_ms);

/**
 * @brief Get monitoring task status
 */
esp_err_t system_metrics_get_monitor_status(bool *running, uint32_t *interval_ms);

/**
 * @brief Configure minimum free heap threshold
 */
esp_err_t system_metrics_set_heap_threshold(size_t threshold_bytes);

/**
 * @brief Get configured minimum free heap threshold
 */
size_t system_metrics_get_heap_threshold(void);

#ifdef __cplusplus
}
#endif

