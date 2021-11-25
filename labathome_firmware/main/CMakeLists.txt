idf_component_register(
    SRCS "main.cc" "plcmanager.cc" "http_handlers.cc" "spiffs.cc"
    EMBED_FILES ../../labathome_webui/dist_compressed/index.html.gz
    )
spiffs_create_partition_image(storage ../spiffs_filesystem FLASH_IN_PROJECT)
