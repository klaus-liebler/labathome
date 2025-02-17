set(USERPROFILE $ENV{USERPROFILE})
idf_component_register(
    SRCS "devicemanager.cc" "main.cc"
    INCLUDE_DIRS "." "../current_board/cpp" "${GENERATED_DIR}"
    REQUIRES "espidf-component-webmanager" "flatbuffers-25.1.24" "lcd_spi_16bpp" "lcd_common" "fatfs" "esp_http_server" "aht_sensor" "common" "driver" "errorcodes" "ds18b20" "ds18b20ext" "espressif__onewire_bus" "bh1750" "ccs811"  "bme280" "vl53l0x" "nau88c22" "i2c_sensor" "audio" "rgbled" "spiffs" "spifilesystem" "pidcontroller" "lsm6ds3"
    EMBED_FILES "../current_board/web/index.compressed.br" "../sounds/de/alarm_co2.mp3" "../sounds/de/alarm_temperature.mp3" "../sounds/de/nok.mp3" "../sounds/de/ok.mp3" "../current_board/sounds/de/ready.mp3" "../sounds/fanfare.mp3" "../sounds/negative.mp3" "../sounds/positive.mp3" "../sounds/siren.mp3"
    EMBED_TXTFILES "../current_board/certificates/esp32.pem.crt" "../current_board/certificates/esp32.pem.key" "${USERPROFILE}/netcase/certificates/rootCA.pem.crt"
    
    )
    
littlefs_create_partition_image(storage ../littlefs_filesystem FLASH_IN_PROJECT)

