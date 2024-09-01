idf_component_register(
    INCLUDE_DIRS "cpp"
    REQUIRES "main" "aht_sensor" "common" "driver" "errorcodes" "ds18b20" "esp_http_server" "bme280"  "i2c_sensor" "audio" "rgbled" "spiffs" "pidcontroller"
    EMBED_FILES "./web/dist/index.compressed.br"
    EMBED_TXTFILES "certificates/esp32.pem.crt" "certificates/esp32.pem.key" "certificates/rootCA.pem.crt"
    )