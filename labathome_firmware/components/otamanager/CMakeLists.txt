idf_component_register(
    SRCS "otamanager.cc"
    INCLUDE_DIRS "include"
    EMBED_TXTFILES sciebo.cer
    REQUIRES nvs_flash esp_https_ota esp_http_client app_update driver
    )
