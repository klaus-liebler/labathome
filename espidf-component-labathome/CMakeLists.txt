idf_component_register(
    INCLUDE_DIRS "cpp" "currentBoardFiles/cpp"
    SRCS "cpp/webmanager.cc"
    REQUIRES "flatbuffers-23.5.26" "main" "aht_sensor" "common" "driver" "errorcodes" "ds18b20" "esp_http_server" "bme280"  "i2c_sensor" "audio" "rgbled" "spiffs" "pidcontroller" "lsm6ds3"
    EMBED_FILES "./currentBoardFiles/web/index.compressed.br" "./currentBoardFiles/sounds_de/alarm_co2.mp3" "./currentBoardFiles/sounds_de/alarm_temperature.mp3" "./currentBoardFiles/sounds_de/nok.mp3" "./currentBoardFiles/sounds_de/ok.mp3" "./currentBoardFiles/sounds_de/ready.mp3" "./music/fanfare.mp3" "./music/negative.mp3" "./music/positive.mp3" "./music/siren.mp3"
    EMBED_TXTFILES "./currentBoardFiles/certificates/esp32.pem.crt" "./currentBoardFiles/certificates/esp32.pem.key" "./certificates/rootCA.pem.crt"
    )