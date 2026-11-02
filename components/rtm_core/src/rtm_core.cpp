#include "rtm/core/rtm_core.hpp"
#include "rtm/core/module_manager.hpp"
#include "esp_log.h"

namespace rtm::core {
    static const char *TAG = "RTM_Core";
    static ModuleManager module_manager;
    static bool core_initialized = false;

    esp_err_t initialize() {
        if (core_initialized) {
            //ESP_LOGW(TAG, "RTM core is already initialized");
            return ESP_OK;
        }
        ESP_LOGI(TAG, "Initializing RTM core");
        core_initialized = true;
        return ESP_OK;
    }

    esp_err_t register_module(Module& module) {
        if (!core_initialized) {
            //ESP_LOGE(TAG, "RTM core must be initialized before registering modules");
            return ESP_ERR_INVALID_STATE;
        }
        return module_manager.register_module(module);
    }

    esp_err_t initialize_modules() {
        if (!core_initialized) {
            return ESP_ERR_INVALID_STATE;
        }
        return module_manager.initialize_all_modules();
    }

    esp_err_t start_modules() {
        if (!core_initialized) {
            return ESP_ERR_INVALID_STATE;
        }
        return module_manager.start_all_modules();
    }

    esp_err_t stop_modules() {
        if (!core_initialized) {
            return ESP_ERR_INVALID_STATE;
        }
        return module_manager.stop_all_modules();
    }

    esp_err_t get_module_state(const char* module_name, ModuleState* state) {
        if (!core_initialized) {
            return ESP_ERR_INVALID_STATE;
        }
        return module_manager.get_module_state(module_name, state);
    }

} // namespace rtm::core


