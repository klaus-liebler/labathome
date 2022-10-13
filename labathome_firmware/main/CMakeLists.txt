idf_component_register(
    SRCS "main.cc"
    INCLUDE_DIRS "."
    REQUIRES "smopla" "esp_adc_cal" "esp_eth" "otamanager" "spifilesystem"
    )
spiffs_create_partition_image(storage ../spiffs_filesystem FLASH_IN_PROJECT)
