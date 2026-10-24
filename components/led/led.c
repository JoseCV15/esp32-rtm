#include "led.h"
#include "driver/gpio.h"
#include "esp_check.h"

static const gpio_num_t LED_GPIO = GPIO_NUM_4;
static bool led_initialize = false;
static bool led_state = false;

esp_err_t led_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << LED_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_RETURN_ON_ERROR(gpio_config(&io_conf), "led", "Failed to configure LED GPIO");
    ESP_RETURN_ON_ERROR(gpio_set_level(LED_GPIO, 0), "led", "Failed to initialize LED state");

    led_state = false;
    led_initialize = true;

    return ESP_OK;
}

esp_err_t led_set(bool state)
{
    if (!led_initialize) {
        return ESP_ERR_INVALID_STATE;
    }
    esp_err_t ret = gpio_set_level(LED_GPIO, state ? 1 : 0);
    if (ret == ESP_OK) {
        led_state = state;
    }
    return ret;
}

esp_err_t led_toggle(void)
{
    if (!led_initialize) {
        return ESP_ERR_INVALID_STATE;
    }
    return led_set(!led_state);
}
