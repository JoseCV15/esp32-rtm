#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <stdbool.h>
#include "esp_err.h"

esp_err_t led_controller_init(void);
esp_err_t led_controller_set_on(void);
esp_err_t led_controller_set_off(void);
esp_err_t led_controller_toggle_for(uint32_t seconds);
esp_err_t led_controller_get_status(bool *is_on, uint32_t *remaining_seconds);



#endif
