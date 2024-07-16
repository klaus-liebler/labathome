idf_component_register(
    SRCS "cpp/devicemanager.cc" "cpp/http_handlers.cc"
    INCLUDE_DIRS "cpp"
    REQUIRES "main" "aht_sensor" "common" "driver" "errorcodes" "ds18b20" "esp_http_server" "bh1750" "bme280" "ccs811" "i2c" "i2c_sensor" "audio" "rgbled" "spiffs" "pidcontroller"
    EMBED_FILES ./web/dist/index.compressed.br
    )