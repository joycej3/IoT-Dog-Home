idf_component_register(
    INCLUDE_DIRS .
    SRCS stepper.c
    REQUIRES driver esp_wifi esp_system nvs_flash
)
