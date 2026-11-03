#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "esp_err.h"
#include "esp_system.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /* System */
    uint64_t uptime;
    esp_reset_reason_t rst_reason;
    uint32_t task_count;
    uint32_t cpu_core_count;

    /* Heap bytes*/
    size_t total_heap;
    size_t free_heap;
    size_t minimum_free_heap;
    size_t largest_free_block;
    //float heap_fragmentation_percent;

    /* RAM Internal */
    size_t total_iram_heap;
    size_t free_iram_heap;
    size_t min_free_iram_heap;
    size_t largest_iram_free_block;

    /* External RAM /PSRAM */

} rtm_metrics_t;

/**
 * @brief Initialize the metrics
 */
esp_err_t rtm_metrics_init(void);

/**
 * @brief Collect a snapshot of current metrics
 * @param metrics  Pointer to the metrics structure
 */
esp_err_t rtm_metrics_get(rtm_metrics_t *metrics);

/**
 * @brief Print current metrics
 */
void rtm_metrics_print(void);

/**
 * @brief Start metrics monitoring
 * @param period_ms Monitoring period in ms
 */
esp_err_t rtm_metrics_start(uint32_t period_ms);

/**
 * @brief Stop metrics monitoring
 */
esp_err_t rtm_metrics_stop(void);

/**
 * @brief Set new monitoring period
 * @param period_ms Monitoring period in ms
 */
esp_err_t rtm_metrics_set_period(uint32_t period_ms);

/**
 * @brief Get the current status of metrics monitoring
 */
esp_err_t rtm_metrics_get_status(bool *running, uint32_t *period_ms);


#ifdef __cplusplus
}
#endif
