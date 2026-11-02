#include "rtm/core/module_manager.hpp"

#include <cstring>
#include "esp_log.h"

namespace rtm::core {
    static const char *TAG = "RTM_ModuleManager";

    esp_err_t ModuleManager::register_module(Module& module) {
        const char* module_name = module.name();
        if (initialized_) {
            ESP_LOGE(TAG, "Cannot register module %s after initialization", module_name);
            return ESP_ERR_INVALID_STATE;
        }

        if (module_name == nullptr || std::strlen(module_name) == 0) {
            ESP_LOGE(TAG, "Module name is invalid");
            return ESP_ERR_INVALID_ARG;
        }

        if (module_count_ >= kMaxModules) {
            ESP_LOGE(TAG, "Cannot register module anymore %zu/%zu", module_count_, kMaxModules);
            return ESP_ERR_NO_MEM;
        }

        if (find_module_entry(module_name) != nullptr) {
            ESP_LOGE(TAG, "Module %s is already registered", module_name);
            return ESP_ERR_INVALID_STATE;
        }

        modules_[module_count_].module = &module;
        modules_[module_count_].state = ModuleState::Uninitialized;
        module_count_++;

        ESP_LOGI(TAG, "Module %s registered successfully", module_name);
        return ESP_OK;
    }

    esp_err_t ModuleManager::initialize_all_modules() {
        if (initialized_) {
            ESP_LOGE(TAG, "Cannot initialize modules after being initialized");
            return ESP_ERR_INVALID_STATE;
        }

        for (std::size_t m = 0; m < module_count_; ++m) {
            ModuleEntry& entry = modules_[m];
            // Dont re-initialize modules that are already initialized or running
            if ((entry.state == ModuleState::Initialized) || (entry.state == ModuleState::Running)) {
                continue;
            }

            // An error in a module requires explicit handling
            if (entry.state == ModuleState::Error) {
                ESP_LOGE(TAG, "Module %s is in error state, cannot initialize", entry.module->name());
                update_manager_state();
                return ESP_FAIL;
            }

            ESP_LOGI(TAG, "Initializing module %s", entry.module->name());

            const esp_err_t err = entry.module->init();
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to initialize module %s, error: %d", entry.module->name(), err);
                entry.state = ModuleState::Error;
                update_manager_state();
                return err;
            }

            entry.state = ModuleState::Initialized;
            ESP_LOGI(TAG, "Module %s initialized successfully", entry.module->name());
        }

        initialized_ = true;
        update_manager_state();
        ESP_LOGI(TAG, "All modules initialized successfully");
        return ESP_OK;
    }

    esp_err_t ModuleManager::start_all_modules() {
        if (!initialized_) {
            ESP_LOGE(TAG, "Modules must be initialized first before starting");
            return ESP_ERR_INVALID_STATE;
        }

        if (running_) {
            ESP_LOGW(TAG, "Modules are already running");
            return ESP_OK;
        }

        for (std::size_t m = 0; m < module_count_; ++m) {
            const ModuleState state = modules_[m].state;
            if (state != ModuleState::Initialized && state != ModuleState::Stopped) {
                ESP_LOGE(TAG, "Module %s is not in a valid state to start (state: %d)", modules_[m].module->name(), static_cast<int>(state));
                return ESP_ERR_INVALID_STATE;
            }
        }

        ESP_LOGI(TAG, "Starting modules");

        std::size_t started_count = 0;

        for (std::size_t m = 0; m < module_count_; ++m) {
            ModuleEntry& entry = modules_[m];
            ESP_LOGI(TAG, "Starting module %s", entry.module->name());

            const esp_err_t err = entry.module->start();
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to start module %s, error: %d", entry.module->name(), err);
                entry.state = ModuleState::Error;
                // Rollback to the modules that started successfully
                for (std::size_t rollback_m = 0; rollback_m < started_count; ++rollback_m) {
                    ModuleEntry& rollback_entry = modules_[rollback_m];
                    ESP_LOGI(TAG, "Stopping module %s due to start failure", rollback_entry.module->name());
                    const esp_err_t rollback_err = rollback_entry.module->stop();
                    if (rollback_err == ESP_OK) {
                        rollback_entry.state = ModuleState::Stopped;
                        ESP_LOGI(TAG, "Module %s stopped successfully", rollback_entry.module->name());
                    }
                    else {
                        rollback_entry.state = ModuleState::Error;
                        ESP_LOGE(TAG, "Failed to stop module %s during rollback, error: %d", rollback_entry.module->name(), rollback_err);
                    }
                }
                update_manager_state();
                return err;
            }

            entry.state = ModuleState::Running;
            started_count++;
            ESP_LOGI(TAG, "Module %s running", entry.module->name());
        }

        update_manager_state();
        ESP_LOGI(TAG, "All modules started");
        return ESP_OK;
    }

    esp_err_t ModuleManager::stop_all_modules() {
        if (!running_) {
            ESP_LOGW(TAG, "no modules running to stop");
            return ESP_OK;
        }

        ESP_LOGI(TAG, "Stopping modules");

        esp_err_t first_error_found = ESP_OK;

        for (std::size_t m = 0; m < module_count_; ++m) {
            ModuleEntry& entry = modules_[m];

            if (entry.state != ModuleState::Running) {
                continue;
            }

            ESP_LOGI(TAG, "Stopping module %s", entry.module->name());

            const esp_err_t err = entry.module->stop();

            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to stop module %s, error: %d", entry.module->name(), err);
                entry.state = ModuleState::Error;
                if (first_error_found == ESP_OK) {
                    first_error_found = err;
                }
            }
            else {
                entry.state = ModuleState::Error;
                ESP_LOGE(TAG, "Module %s stopped with error", entry.module->name());
            } 
        }
        
        update_manager_state();

        if (first_error_found != ESP_OK) {
            ESP_LOGE(TAG, "Some modules failed to stop");
            return first_error_found;
        }

        ESP_LOGI(TAG, "All modules stopped");
        return ESP_OK;
    }

    
    esp_err_t ModuleManager::get_module_state(const char* module_name, ModuleState* state) const {
        if (module_name == nullptr || state == nullptr) {
            return ESP_ERR_INVALID_ARG;
        }

        const ModuleEntry* entry = find_module_entry(module_name);
        if (entry == nullptr) {
            return ESP_ERR_NOT_FOUND;
        }

        *state = entry->state;

        return ESP_OK;
    }

    ModuleManager::ModuleEntry* ModuleManager::find_module_entry(const char *module_name) {
        if (module_name == nullptr) {
            return nullptr;
        }

        for (std::size_t m = 0; m < module_count_; ++m) {
            if (std::strcmp(modules_[m].module->name(), module_name) == 0) {
                return &modules_[m];
            }
        }
        return nullptr;
    }

    const ModuleManager::ModuleEntry* ModuleManager::find_module_entry(const char *module_name) const {
        if (module_name == nullptr) {
            return nullptr;
        }
        for (std::size_t m = 0; m < module_count_; ++m) {
            if (std::strcmp(modules_[m].module->name(), module_name) == 0) {
                return &modules_[m];
            }
        }

        return nullptr;
    }

    void ModuleManager::update_manager_state() {
        initialized_ = true;
        running_ = true;

        for (std::size_t m = 0; m < module_count_; ++m) {
            const ModuleState state = modules_[m].state;
            if (state == ModuleState::Uninitialized || state == ModuleState::Error) {
                initialized_ = false;
            }
            if (state != ModuleState::Running) {
                running_ = false;
            }
        }

        if (module_count_ == 0) {
            initialized_ = false;
            running_ = false;
        }
    }

    std::size_t ModuleManager::get_module_count() const {
        return module_count_;
    }

    bool ModuleManager::is_module_initialized() const {
        return initialized_;
    }

    bool ModuleManager::is_module_running() const {
        return running_;
    }     
} // namespace rtm::core








