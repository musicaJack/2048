idf_component_register(
    SRCS "ili9488_panel.cpp"
    INCLUDE_DIRS "include"
    REQUIRES driver esp_driver_gpio esp_driver_spi
)
