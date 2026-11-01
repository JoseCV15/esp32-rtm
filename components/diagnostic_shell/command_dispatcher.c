#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "esp_console.h"
#include "esp_err.h"
#include "esp_log.h"

#include "command_dispatcher.h"
#include "led_controller.h"
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
    esp_err_t err = ESP_OK;

    if (argc < 2) {
        printf("LED commands:\n");
        printf("  led on\n");
        printf("  led off\n");
        printf("  led toggle <seconds>\n");
        printf("  led status\n");
        return 0;
    }

    if (strcmp(argv[1], "on") == 0) {
        if (argc != 2) {
            printf("Usage led on\n");
            return 1;
        }
        err = led_controller_set_on();
    }
    else if (strcmp(argv[1], "off") == 0) {
        if (argc != 2) {
            printf("Usage led off\n");
            return 1;
        }
        err = led_controller_set_off();
    }
    else if (strcmp(argv[1], "toggle") == 0) {
        if (argc != 3) {
            printf("Usage led toggle <seconds>\n");
            return 1;
        }

        char *endptr = NULL;
        unsigned long seconds = strtoul(argv[2], &endptr, 10);

        if (argv[2][0] == '\0' || *endptr != '\0' || seconds == 0 || seconds > 3600) {
            printf("Invalid duration. Must be between 1 and 3600 seconds\n");
            return 1;
        }

        err = led_controller_toggle_for((uint32_t)seconds);

        if (err == ESP_OK) {
            printf("LED ON for %lu seconds\n", seconds);
        }
    }
    else if (strcmp(argv[1], "status") == 0) {
        if (argc != 2) {
            printf("Usage: led status\n");
            return 1;
        }

        bool is_on = false;
        uint32_t remaining_seconds = 0;

        err = led_controller_get_status(&is_on, &remaining_seconds);
        if (err == ESP_OK) {
            printf("\n===== LED STATUS =====\n");
            printf("State: %s\n", is_on ? "ON" : "OFF");

            if (remaining_seconds > 0) {
                printf("Remaining time: %lu seconds\n",(unsigned long)remaining_seconds);
            } else {
                printf("Timer: inactive\n");
            }

            printf("======================\n\n");
        }
    }
    else {
        printf("Unknown Led command: %s\n", argv[1]);
        return 1;
    }

    if (err != ESP_OK) {
        printf("Led command failed: %s\n", esp_err_to_name(err));
        return 1;
    }
    
    return 0;
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
* Configuration cmd
* monitor status
* monitor threshold <KB>
* monitor interval <ms>
* monitor start
* montior stop
*/

static int cmd_monitor_configuration(int argc, char *argv[])
{
    esp_err_t err = ESP_OK;
    if (argc < 2) {
        printf("\nMonitor commands:\n");
        printf("  monitor status\n");
        printf("  monitor threshold <KB>\n");
        printf("  monitor interval <ms>\n");
        printf("  monitor start\n");
        printf("  monitor stop\n\n");
        return 0;
    }

    // Monitor Status
    if (strcmp(argv[1], "status") == 0) {
        if (argc != 2) {
            printf("Usage: monitor status\n");
            return 1;
        }
        bool running = false;
        uint32_t interval = 0;
        err = system_metrics_get_monitor_status(&running, &interval);
        if (err != ESP_OK) {
            goto monitor_error;
        }

        printf("\n");
        printf("======== MONITOR STATUS ========\n");
        printf("State: %s\n", running ? "RUNNING" : "STOPPED");
        printf("Interval: %lu ms\n", (unsigned long)interval);
        printf("Heap threshold: %u KB\n", (unsigned int)(system_metrics_get_heap_threshold() / 1024));
        printf("================================\n\n");
        return 0;
    }

    else if (strcmp(argv[1], "threshold") == 0) {
        if (argc != 3) {
            printf("Usage: monitor threshold <KB>\n");
            return 1;
        }

        char *endptr = NULL;
        unsigned long kb = strtoul(argv[2], &endptr, 10);

        if (argv[2][0] == '\0' || *endptr != '\0' || kb == 0 || kb > SIZE_MAX / 1024) {
            printf("Invalid threshold value\n");
            return 1;
        }

        size_t threshold_bytes = (size_t)kb * 1024;
        err = system_metrics_set_heap_threshold(threshold_bytes);
        if (err != ESP_OK) {
            goto monitor_error;
        }
        printf("Heap threshold set to %lu KB\n", kb);
        return 0;
    }
    // interval
    else if (strcmp(argv[1], "interval") == 0) {
        if (argc != 3) {
            printf("Usage: monitor interval <ms>\n");
            return 1;
        }

        char *endptr = NULL;
        unsigned long ms = strtoul(argv[2], &endptr, 10);

        if (argv[2][0] == '\0' || *endptr != '\0' || ms < 100 || ms > 60000) {
            printf("Invalid interval. Must be between 1oo and 60000 ms\n");
            return 1;
        }

        err = system_metrics_set_heap_threshold((uint32_t)ms);
        if (err != ESP_OK) {
            goto monitor_error;
        }
        printf("Heap interval set to %lu ms\n", ms);
        return 0;
    }

    // Monitor start
    else if (strcmp(argv[1], "start") == 0) {
        if (argc != 2) {
            printf("Usage: monitor start\n");
            return 1;
        }

        err = system_metrics_start_monitor(0);
        if (err != ESP_OK) {
            goto monitor_error;
        }
        printf("Monitor started\n");
        return 0;
    }
    // Monitor stop
    else if (strcmp(argv[1], "stop") == 0) {
        if (argc != 2) {
            printf("Usage: monitor stop\n");
            return 1;
        }

        err = system_metrics_stop_monitor();
        if (err != ESP_OK) {
            goto monitor_error;
        }
        printf("Monitor stopped\n");
        return 0;
    }
    else {
        printf("Unknown Monitor action: %s\n", argv[1]);
        printf("Type 'monitor' to see available commands\n");
        return 1;
    }

monitor_error:
    printf("Monitor command failed: %s\n", esp_err_to_name(err));
    return 1;
    
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

    err = register_cmd("status", "Display system status", NULL, cmd_status);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register status command: %s", esp_err_to_name(err));
        return err;
    }

    err = register_cmd("metrics", "Display system metrics", NULL, cmd_metrics);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register metrics command: %s", esp_err_to_name(err));
        return err;
    }

    err = register_cmd("monitor", "Configure system monitoring", NULL, cmd_monitor_configuration);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register monitor command: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "All diagnostic commands registered");
    return ESP_OK;
}
