#include "rtm/core/rtm_core.hpp"
#include "rtm/modules/led_module.hpp"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "RTM_Main";

extern "C" void app_main(void) {
    static rtm::modules::LedModule led_module;

    ESP_LOGI(TAG, "Starting ESP32-RTM");

    // Initialize RTM Core
    rtm::core::initialize();
    // Register Led module
    rtm::core::register_module(led_module);
    // Initialize module
    rtm::core::initialize_modules();
    // Start module
    rtm::core::start_modules();

    ESP_LOGI(TAG, "ESP32-RTM started successfully");

    led_module.set_on();

    vTaskDelay(pdMS_TO_TICKS(2000));

    led_module.set_off();

}