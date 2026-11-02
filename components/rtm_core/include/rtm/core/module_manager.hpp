#pragma once

#include <array>
#include <cstddef>

#include "esp_err.h"
#include "rtm/core/module.hpp"

namespace rtm::core {

    class ModuleManager {
    public:
        static constexpr std::size_t kMaxModules = 10;

        ModuleManager() = default;
        ~ModuleManager() = default;

        ModuleManager(const ModuleManager&) = delete;
        ModuleManager& operator=(const ModuleManager&) = delete;

        // Register a module with the manager
        esp_err_t register_module(Module& module);
        // Initialize all registered modules
        esp_err_t initialize_all_modules();
        // Start all registered modules
        esp_err_t start_all_modules();
        // Stop all running modules
        esp_err_t stop_all_modules();
        // Get the state of a module by name
        esp_err_t get_module_state(const char* module_name, ModuleState* state) const;

        std::size_t get_module_count() const;
        bool is_module_initialized() const;
        bool is_module_running() const;
    
    private:
        struct ModuleEntry {
            Module* module;
            ModuleState state = ModuleState::Uninitialized;
        };

        std::array<ModuleEntry, kMaxModules> modules_;
        std::size_t module_count_ = 0;
        bool initialized_ = false;
        bool running_ = false;

        ModuleEntry* find_module_entry(const char* module_name);
        const ModuleEntry* find_module_entry(const char* module_name) const;

        void update_manager_state();
    };

} // namespace rtm::core