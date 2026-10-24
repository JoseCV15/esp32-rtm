idf_component_register(
    SRCS "system_metrics.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_timer esp_system heap log
)
