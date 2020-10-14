idf_component_register(
    SRCS "main.cc" "plcmanager.cc" "http_handlers.cc" "spiffs.cc" "connect.c" "PID_v1.cpp"
    )
spiffs_create_partition_image(spiffs data FLASH_IN_PROJECT)