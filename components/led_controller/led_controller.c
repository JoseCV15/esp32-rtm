#include <stdio.h>
#include "led_controller.h"
#include "led.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

static const char *TAG = "LED_CONTROLLER";

static TimerHandle_t led_timer = NULL;
static  bool initialized = false;

/*
* Called automatically when the timer expires.
*/

static void led_timer_callback(TimerHandle_t timer)
{
    (void)timer;
    esp_err_t err = led_set(false);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Timed LED operation finished. LED OFF");
    }
    else {
        ESP_LOGE(TAG, "Failed to turn LED OFF: %s", esp_err_to_name(err));
    }
}

/*
* Initialize the LED controller
* Must be called after led_init
*/
esp_err_t led_controller_init(void)
{
    if (initialized) {
        return ESP_OK;
    }

    led_timer = xTimerCreate(
        "led_timer",
        pdMS_TO_TICKS(1000),
        pdFALSE,
        NULL,
        led_timer_callback
    );
    if (led_timer == NULL) {
        ESP_LOGE(TAG, "Failed to create LED timer");
        return ESP_ERR_NO_MEM;
    }
    initialized = true;
    ESP_LOGI(TAG, "LED controller initialized");
    return ESP_OK;
}

/*
* Turn LED ON
* Any active timer is cancelled
*/
esp_err_t led_controller_set_on(void)
{
    if (!initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (xTimerStop(led_timer, 0) != pdPASS) {
        return ESP_FAIL;
    }

    return led_set(true);
}

/*
* Turn LED OFF
*/
esp_err_t led_controller_set_off(void)
{
    if (!initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (xTimerStop(led_timer, 0) != pdPASS) {
        return ESP_FAIL;
    }

    return led_set(false);
}

/*
* Turn LED ON for a specified duration
* A new call replaces the previous timer
*/
esp_err_t led_controller_toggle_for(uint32_t seconds)
{
    if (!initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    if (seconds == 0 || seconds > 3600) {
        return ESP_ERR_INVALID_ARG;
    }

    TickType_t duration_ticks = pdMS_TO_TICKS(seconds * 1000U);
    if (duration_ticks == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    // Cancel the previous timer
    if (xTimerStop(led_timer, 0) != pdPASS) {
        return ESP_FAIL;
    }

    // Turn LED ON
    esp_err_t err = led_set(true);
    if (err != ESP_OK) {
        return err;
    }

    if (xTimerChangePeriod(led_timer, duration_ticks, 0) != pdPASS) {
        led_set(false);

        ESP_LOGE(TAG, "Failed to configure LED timer");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "LED ON for %lu seconds", (unsigned long)seconds);

    return ESP_OK;
}

/**
* @brief Get current LED state and remaining timer duration
* @param remaining_seconds = 0 means no active timer
*/
esp_err_t led_controller_get_status(bool *is_on, uint32_t *remaining_seconds)
{
    if (!initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    if (is_on == NULL || remaining_seconds == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = led_get_state(is_on);

    if (err != ESP_OK) {
        return err;
    }

    *remaining_seconds = 0;
    if (xTimerIsTimerActive(led_timer) == pdTRUE) {
        TickType_t expiry = xTimerGetExpiryTime(led_timer);
        TickType_t now = xTaskGetTickCount();
        if (expiry > now) {
            TickType_t remaining_ticks = expiry - now;
            *remaining_seconds = (uint32_t)((remaining_ticks * portTICK_PERIOD_MS + 999) / 1000);
        }
    }

    return ESP_OK;
}
