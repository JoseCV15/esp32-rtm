#include "rtm/modules/led_module.hpp"
#include "led.h"
#include "esp_log.h"

namespace rtm::modules {
    static const char *TAG = "RTM_LedModule";

    esp_err_t LedModule::init() {
        ESP_LOGI(TAG, "Initializing Led Module");
        return led_init();
    }

    esp_err_t LedModule::start() {
        ESP_LOGI(TAG, "Starting Led Module");
        return ESP_OK;
    }

    esp_err_t LedModule::stop() {
        ESP_LOGI(TAG, "Stopping Led Module");
        return led_set(false);
    }

    esp_err_t LedModule::set_on() {
        ESP_LOGI(TAG, "LED ON");
        return led_set(true);
    }
    
    esp_err_t LedModule::set_off() {
        ESP_LOGI(TAG, "LED OFF");
        return led_set(false);
    }

    const char* LedModule::name() const {
        return TAG;
    }

}