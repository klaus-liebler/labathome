cmake_minimum_required(VERSION 3.30)
set(PROJECT_VER "0.0.0.4")

# Von builder (C#) erzeugt: dotnet run --project builder -- Pipeline
set(GENERATED_DIR "${CMAKE_CURRENT_LIST_DIR}/generated/")
set(USERPROFILE $ENV{USERPROFILE})
set(EXTRA_COMPONENT_DIRS "../../espidf-components" "../../espidf-component-webmanager" "../../espidf-components-lcd")

file(READ "${GENERATED_DIR}/cmake/config.json" config_json)
string(JSON HOSTNAME GET "${config_json}" HOSTNAME)
string(JSON BOARD_NAME GET "${config_json}" BOARD_NAME)
string(JSON BOARD_VERSION GET "${config_json}" BOARD_VERSION)
string(JSON BOARD_HAL_DIR GET "${config_json}" BOARD_HAL_DIR)
string(JSON BOARD_MAC GET "${config_json}" BOARD_MAC)
string(JSON APP_NAME GET "${config_json}" APP_NAME)
string(JSON APP_VERSION GET "${config_json}" APP_VERSION)
string(JSON CREATION_DT GET "${config_json}" CREATION_DT)
string(JSON CREATION_DT_STR GET "${config_json}" CREATION_DT_STR)
string(JSON GIT_SHORT_HASH GET "${config_json}" GIT_SHORT_HASH)
string(JSON BOARD_DIRECTORY GET "${config_json}" BOARD_DIRECTORY)
string(JSON CERTS_DIR GET "${config_json}" CERTS_DIR)

message(STATUS "Build for BOARD ${BOARD_NAME} ${BOARD_VERSION} (HAL ${BOARD_HAL_DIR}) with MAC ${BOARD_MAC}")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(labathome_firmware)
