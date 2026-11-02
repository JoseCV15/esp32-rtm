#pragma once

#include "esp_err.h"
#include "rtm/core/module.hpp"

namespace rtm::core {
    // Initialize core
    esp_err_t initialize();
    // Register module
    esp_err_t register_module(Module& module);
    // Initialize registered modules
    esp_err_t initialize_modules();

    // Start modules
    esp_err_t start_modules();
    esp_err_t stop_modules();
    esp_err_t get_module_state(const char* module_name, ModuleState* state);
} // namespace rtm::core
