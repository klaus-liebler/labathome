#pragma once
#include <stdint.h>
#include <errorcodes.hh>
#include <common.hh>
#include "esp_vfs.h"
#include "esp_spiffs.h"

#define LABATHOME_V10 10
#define PTNCHEN_V2 1002
#define TARGET LABATHOME_V10

namespace labathome::config{
    constexpr size_t HTTP_SCRATCHPAD_SIZE{2048};
#if TARGET==LABATHOME_V10
    constexpr char OTA_URL[]{"https://w-hs.sciebo.de/s/M43yoLOweqCjl5v/download"};
#elif TARGET==PTNCHEN_V2
    constexpr char OTA_URL[]{"https://w-hs.sciebo.de/s/M43yoLOweqCjl5v/download"};//TODO
#else
    #error "No OTA URL for this target defined. See common_projectconfig.hh"
#endif
}



namespace labathome::config::paths{
    constexpr size_t FILE_PATH_MAX =ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN;
    constexpr const char* FBDSTORE_BASE = "/spiffs/fbd/";
    constexpr const char* FBDSTORE_BASE_DIRECTORY = "/spiffs/fbd";
    
    constexpr const char *DEFAULT_FBD_BIN_FILENAME =  "/spiffs/defaultfbd.bin";
    constexpr const char *DEFAULT_FBD_JSON_FILENAME = "/spiffs/defaultfbd.json";
}