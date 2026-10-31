#include <stdio.h>
#include "diagnostic_shell.h"
#include "command_dispatcher.h"

#include "esp_console.h"
#include "esp_log.h"

static const char *TAG = "diagnostic_shell";

esp_err_t diagnostic_shell_start(void)
{
    esp_err_t err;

    // Register commands before starting REPL
    err = command_dispatcher_register_commands();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Command registration failed: %s",
                 esp_err_to_name(err));
        return err;
    }

    // Register help cmd
    err = esp_console_register_help_command();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Help registration failed: %s", esp_err_to_name(err));
        return err;
    }

    // Configure REPL
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = "esp32-rtm> ";
    repl_config.max_cmdline_length = 256;
    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    esp_console_repl_t *repl = NULL;

    // Create REPL UART
    err = esp_console_new_repl_uart(&uart_config, &repl_config, &repl);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create UART REPL: %s", esp_err_to_name(err));
        return err;
    }

    // Start REPL
    err = esp_console_start_repl(repl);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start REPL: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "Diagnostic shell started");

    return ESP_OK;
}

