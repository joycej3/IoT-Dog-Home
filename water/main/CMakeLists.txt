idf_component_register(SRCS "continuous_read_main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES driver unity servo esp_adc)
                    # INCLUDE_DIRS "." PRIV_REQUIRES unity test_utils servo)
