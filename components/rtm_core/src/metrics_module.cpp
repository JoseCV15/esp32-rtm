#include "rtm/modules/metrics_module.hpp"
#include "rtm_metrics.h"
#include "esp_log.h"

namespace rtm::modules {
    static const char *TAG = "RTM_MetricsModule";
    static constexpr uint32_t METRICS_INTERVAL_MS = 5000;

    esp_err_t SystemMetricsModule::init() {
        ESP_LOGI(TAG, "Initializing Metrics Module");
        return rtm_metrics_init();
    }

    esp_err_t SystemMetricsModule::start() {
        ESP_LOGI(TAG, "Starting Metrics Module");
        return rtm_metrics_start(METRICS_INTERVAL_MS);
    }

    esp_err_t SystemMetricsModule::stop() {
        ESP_LOGI(TAG, "Stopping Metrics Module");
        return rtm_metrics_stop();
    }

    const char* SystemMetricsModule::name() const {
        return TAG;
    }
}
