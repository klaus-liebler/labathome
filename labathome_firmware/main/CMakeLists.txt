idf_component_register(
    SRCS "main.cc"
    INCLUDE_DIRS "."
    )
spiffs_create_partition_image(storage ../spiffs_filesystem FLASH_IN_PROJECT)
