idf_component_register(
    SRCS "main.cc" "plcmanager.cc" "http_handlers.cc" "spiffs.cc" "connect.c" "PID_v1.cpp"
    EMBED_FILES ../../labathome_webui/dist_compressed/index.html.gz
    )
spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)
