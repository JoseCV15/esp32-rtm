#include "rtm/core/rtm_core.hpp"
#include "rtm_metrics.h"
#include "rtm/modules/led_module.hpp"
#include "rtm/modules/metrics_module.hpp"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "RTM_Main";

static rtm::modules::LedModule led_module;
static rtm::modules::SystemMetricsModule metrics_module;

extern "C" void app_main(void) {

    ESP_LOGI(TAG, "Starting ESP32-RTM");

    // Initialize RTM Core
    rtm::core::initialize();
    // Register modules
    rtm::core::register_module(led_module);
    rtm::core::register_module(metrics_module);
    // Initialize module
    rtm::core::initialize_modules();
    // Start module
    rtm::core::start_modules();

    ESP_LOGI(TAG, "ESP32-RTM started successfully");

    // Test led module
    led_module.set_on();
    vTaskDelay(pdMS_TO_TICKS(2000));
    led_module.set_off();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

}