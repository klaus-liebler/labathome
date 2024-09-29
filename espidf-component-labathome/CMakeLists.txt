idf_component_register(
    INCLUDE_DIRS "cpp"
    REQUIRES "main" "aht_sensor" "common" "driver" "errorcodes" "ds18b20" "esp_http_server" "bme280"  "i2c_sensor" "audio" "rgbled" "spiffs" "pidcontroller" "lsm6ds3"
    EMBED_FILES "./web/dist/index.compressed.br" "./speech_de/alarm_co2.mp3" "./speech_de/alarm_temperature.mp3" "./speech_de/nok.mp3" "./speech_de/ok.mp3" "./speech_de/ready.mp3" "./music/fanfare.mp3" "./music/negative.mp3" "./music/positive.mp3" "./music/siren.mp3"
    EMBED_TXTFILES "certificates/esp32.pem.crt" "certificates/esp32.pem.key" "certificates/rootCA.pem.crt"
    )