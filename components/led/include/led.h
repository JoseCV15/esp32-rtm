#ifndef LED_H
#define LED_H

#include <stdbool.h>
#include "esp_err.h"

/* Public API */
esp_err_t led_init(void);
esp_err_t led_set(bool state);
esp_err_t led_toggle(void);

#endif
