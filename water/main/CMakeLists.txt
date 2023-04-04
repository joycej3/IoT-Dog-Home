idf_component_register(SRCS "continuous_read_main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES driver unity esp_adc esp_wifi esp_event nvs_flash protocol_examples_common)
                    # INCLUDE_DIRS "." PRIV_REQUIRES unity test_utils servo)
