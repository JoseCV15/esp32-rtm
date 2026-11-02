#pragma once

#include "rtm/core/module.hpp"

namespace rtm::modules {
    class LedModule : public rtm::core::Module {
    public:
        esp_err_t init() override;
        esp_err_t start() override;
        esp_err_t stop() override;
        esp_err_t set_on();
        esp_err_t set_off();
        const char* name() const override;
    };

}



