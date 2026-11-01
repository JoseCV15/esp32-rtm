idf_component_register(
    SRCS 
        "diagnostic_shell.c" 
        "command_dispatcher.c"

    INCLUDE_DIRS 
        "include"
        
    REQUIRES 
        console
        log
        led_controller
        system_metrics
)
