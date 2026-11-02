#pragma once

#include "esp_err.h"

namespace rtm::core {
    
    enum class ModuleState {
        Uninitialized,
        Initialized,
        Running,
        Stopped,
        Error
    };

    class Module {
        public:
            virtual ~Module() = default;

            // Initialize module resources
            virtual esp_err_t init() = 0;
            // Start module operation
            virtual esp_err_t start() = 0;
            // Stop module operation
            virtual esp_err_t stop() = 0;

            // Return module name
            virtual const char* name() const = 0;
    };
} // namespace rtm::core