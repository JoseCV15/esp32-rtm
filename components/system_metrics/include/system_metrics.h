#ifndef SYSTEM_METRICS_H
#define SYSTEM_METRICS_H

#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

typedef struct {
    uint64_t uptime_ms;
    size_t free_heap_bytes;
    size_t minimum_free_heap_bytes;
    size_t free_internal_heap_bytes;
    bool led_state;
}system_metrics_t;

esp_err_t system_metrics_get(system_metrics_t *self);
void system_metrics_log(void);

esp_err_t system_metrics_start_monitor(uint32_t interval_ms);
esp_err_t system_metrics_stop_monitor(void);
esp_err_t system_metrics_set_interval(uint32_t interval_ms);
esp_err_t system_metrics_get_monitor_status(bool *running, uint32_t *interval_ms);
esp_err_t system_metrics_set_heap_threshold(size_t threshold_bytes);
size_t system_metrics_get_heap_threshold(void);

#endif
