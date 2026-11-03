idf_component_register(
    SRCS 
        "src/rtm_core.cpp"
        "src/module_manager.cpp"
        "src/led_module.cpp"
        "src/metrics_module.cpp"
    INCLUDE_DIRS 
        "include"
    REQUIRES
        esp_common
        log
        led
        rtm_metrics
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
