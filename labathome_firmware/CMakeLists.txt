cmake_minimum_required(VERSION 3.16.0)
set(PROJECT_VER "0.0.0.4")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(EXTRA_COMPONENT_DIRS "../../espidf-components" "../espidf-component-labathome" "../../espidf-component-webmanager")
project(labathome_firmware)
