# Das HAL-Verzeichnis (main/hal/<BOARD_HAL_DIR>) bestimmt der C#-Builder aus der Board-Version
# (zweistellig mit fuehrender Null aufgefuellt: 50100 -> "05"), s. builder/LabathomeBuildContext.cs.
message(STATUS "Including HAL directory hal/${BOARD_HAL_DIR}, derived from BOARD_VERSION ${BOARD_VERSION}")

set(LABATHOME_REQUIRES "espidf-component-webmanager" "lcd_spi_16bpp" "lcd_common" "fatfs" "esp_http_server" "aht_sensor" "common" "driver" "errorcodes" "ds18b20" "ds18b20ext" "espressif__onewire_bus" "bh1750" "ccs811" "bme280" "vl53l0x" "nau88c22" "i2c_sensor" "audio" "led" "spiffs" "spifilesystem" "pidcontroller" "lsm6ds3" "modbus_rtu" "rotenc" "esp_driver_uart" "esp_driver_gpio" "esp_driver_i2c" "esp_driver_spi" "esp_driver_i2s" "esp_driver_dac" "esp_driver_mcpwm" "esp_driver_pcnt" "esp_adc" "esp_timer" "nvs_flash" "esp_wifi" "esp_netif" "esp_https_server")

# USB-OTG/TinyUSB nur auf Targets, die es haben (nicht auf dem klassischen ESP32)
idf_build_get_property(target IDF_TARGET)
if(target STREQUAL "esp32s2" OR target STREQUAL "esp32s3" OR target STREQUAL "esp32p4")
    list(APPEND LABATHOME_REQUIRES "espressif__esp_tinyusb")
endif()

idf_component_register(
    SRCS "devicemanager.cc" "main.cc" "hal/i2c_discover.cc" "modbus.cc"
    INCLUDE_DIRS "." "hal/${BOARD_HAL_DIR}" "${GENERATED_DIR}"
    REQUIRES ${LABATHOME_REQUIRES}
    EMBED_FILES "${GENERATED_DIR}/web/index.compressed.br" "../sounds/de/alarm_co2.mp3" "../sounds/de/alarm_temperature.mp3" "../sounds/de/nok.mp3" "../sounds/de/ok.mp3" "${BOARD_DIRECTORY}/sounds/de/ready.mp3" "../sounds/fanfare.mp3" "../sounds/negative.mp3" "../sounds/positive.mp3" "../sounds/siren.mp3"
    EMBED_TXTFILES "${BOARD_DIRECTORY}/certificates/esp32.pem.crt" "${BOARD_DIRECTORY}/certificates/esp32.pem.key" "${CERTS_DIR}/rootCA.pem.crt"
    )

littlefs_create_partition_image(storage ../littlefs_filesystem FLASH_IN_PROJECT)
