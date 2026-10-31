#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "esp_console.h"
#include "esp_err.h"
#include "esp_log.h"

#include "command_dispatcher.h"
#include "led.h"
#include "system_metrics.h"

static const char *TAG = "command_dispacher";

/*
 * LED CMD
 * Led on
 * Led off
 * Led toggle
 */

static int cmd_led(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: led <on|off|toggle>\n");
        return 1;
    }

    esp_err_t err;
    if (strcmp(argv[1], "on") == 0) {
        err = led_set(true);
    }
    else if (strcmp(argv[1], "off") == 0) {
        err = led_set(false);
    }
    else if (strcmp(argv[1], "toggle") == 0) {
        err = led_toggle();
    }
    else {
        printf("Unknown Led action: %s\n", argv[1]);
        return 1;
    }

    if (err != ESP_OK) {
        printf("Led command failed: %s\n", esp_err_to_name(err));
        return 1;
    }
    // get led status
    bool state = false;
    err = led_get_state(&state);
    if (err == ESP_OK) {
        printf("Led state: %s\n", state ? "ON": "OFF");
    }
    return err == ESP_OK ? 0 : 1;
}

/*
* Status Command
*/

static int cmd_status(int argc, char *argv[])
{
    (void)argv;
    if (argc != 1) {
        printf("Usage: status\n");
        return 1;
    }

    system_metrics_t metrics = {0};

    esp_err_t err = system_metrics_get(&metrics);
    if (err != ESP_OK) {
        printf("Failed to get system metrics: %s\n", esp_err_to_name(err));
        return 1;
    }

    printf("\n");
    printf("======== SYSTEM STATUS ==========\n");
    printf("Uptime:              %llu ms\n", (unsigned long long)metrics.uptime_ms);
    printf("Free heap:           %zu bytes\n", metrics.free_heap_bytes);
    printf("Minimum free heap:   %zu bytes\n", metrics.minimum_free_heap_bytes);
    printf("Free internal heap:  %zu bytes\n", metrics.free_internal_heap_bytes);
    printf("LED state:           %s\n", metrics.led_state ? "ON" : "OFF");
    printf("===================================\n\n");
    return 0;
}

/*
* Metrics command
*/

static int cmd_metrics(int argc, char *argv[])
{
    (void)argv;

    if (argc != 1) {
        printf("Usage: metrics\n");
        return 1;
    }

    system_metrics_log();
    return 0;
}

/*
* Command registration helper
*/

static esp_err_t register_cmd(const char *command, const char *help, const char *hint, esp_console_cmd_func_t func)
{
    const esp_console_cmd_t cmd = {
        .command = command,
        .help = help,
        .hint = hint,
        .func = func
    };
    return esp_console_cmd_register(&cmd);
}

/*
* Register all commands
*/

esp_err_t command_dispatcher_register_commands(void)
{
    esp_err_t err;

    err = register_cmd("led", "Control the LED", "<on|off|toggle>", cmd_led);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register led command: %s", esp_err_to_name(err));
        return err;
    }

    err = register_cmd("status", "Display current system status", NULL, cmd_status);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register status command: %s", esp_err_to_name(err));
        return err;
    }

    err = register_cmd("metrics", "Display system metrics", NULL, cmd_metrics);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register metrics command: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "All diagnostic commands registered");
    return ESP_OK;
}
