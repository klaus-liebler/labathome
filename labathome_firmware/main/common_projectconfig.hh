#pragma once
#include <stdint.h>
#include <errorcodes.hh>
#include <common.hh>
#include "esp_vfs.h"
#include "esp_spiffs.h"

namespace labathome::config{
    constexpr size_t HTTP_SCRATCHPAD_SIZE{2048};
}

namespace labathome::config::paths{
    constexpr size_t FILE_PATH_MAX =ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN;
    constexpr const char* FBDSTORE_BASE = "/spiffs/fbd/";
    constexpr const char* FBDSTORE_BASE_DIRECTORY = "/spiffs/fbd";
    
    constexpr const char *DEFAULT_FBD_BIN_FILENAME =  "/spiffs/defaultfbd.bin";
    constexpr const char *DEFAULT_FBD_JSON_FILENAME = "/spiffs/defaultfbd.json";
}