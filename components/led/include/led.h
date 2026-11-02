#pragma once

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Public API */
esp_err_t led_init(void);
esp_err_t led_set(bool state);
esp_err_t led_toggle(void);
esp_err_t led_get_state(bool *state);

#ifdef __cplusplus
}
#endif
