idf_component_register(
    SRCS "board_input.cpp"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_driver_gpio esp_driver_i2c esp_timer
)
