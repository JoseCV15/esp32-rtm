#pragma once

#include "rtm/core/module.hpp"
#include "esp_err.h"

namespace rtm::modules {
    class SystemMetricsModule : public rtm::core::Module {
    public:
        esp_err_t init() override;
        esp_err_t start() override;
        esp_err_t stop() override;
        const char* name() const override;
    };

} // namespace rtm::modules