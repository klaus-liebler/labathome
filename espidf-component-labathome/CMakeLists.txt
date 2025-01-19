idf_component_register(
    INCLUDE_DIRS "cpp" "currentBoardFiles/cpp"
    SRCS "cpp/webmanager.cc"
    REQUIRES "flatbuffers-23.5.26" "main" "aht_sensor" "common" "driver" "errorcodes" "ds18b20" "esp_http_server" "bme280"  "i2c_sensor" "audio" "rgbled" "spiffs" "pidcontroller" "lsm6ds3"
    EMBED_FILES "./currentBoardFiles/web/index.compressed.br" "./static_sounds/alarm_co2.mp3" "./static_sounds/alarm_temperature.mp3" "./static_sounds/nok.mp3" "./static_sounds/ok.mp3" "./currentBoardFiles/sounds_de/ready.mp3" "./static_sounds/fanfare.mp3" "./static_sounds/negative.mp3" "./static_sounds/positive.mp3" "./static_sounds/siren.mp3"
    EMBED_TXTFILES "./currentBoardFiles/certificates/esp32.pem.crt" "./currentBoardFiles/certificates/esp32.pem.key" "./certificates/rootCA.pem.crt"
    )