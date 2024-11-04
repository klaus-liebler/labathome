import { IDF_PATH } from "./gulpfile_config";
import path from "node:path";



export const ROOT = "..";


//top level directories
export const WEB = path.join(ROOT, "web");

//Board Specific
export const BOARDS_BASE_DIR="../boards";
export const SOUNDS_DE_SUBDIR = "sounds_de";
export const CERTIFICATES_SUBDIR = "certificates";
export const ESP32_CERT_PEM_CRT_FILENAME = "esp32.pem.crt";
export const ESP32_CERT_PEM_PRVTKEY_FILENAME = "esp32.pem.key";
export const ESP32_CERT_PEM_PUBKEY_FILENAME = "esp32.pem.pubkey";

//various servers
export const TESTSERVER = path.join(ROOT, "testserver");
export const CERTIFICATES = path.join(ROOT, "certificates");
export const ROOT_CA_PEM_CRT = path.join(CERTIFICATES, "rootCA.pem.crt");
export const ROOT_CA_PEM_PRVTKEY = path.join(CERTIFICATES, "rootCA.pem.key");
export const TESTSERVER_CERT_PEM_CRT = path.join(CERTIFICATES, "testserver.pem.crt");
export const TESTSERVER_CERT_PEM_PRVTKEY = path.join(CERTIFICATES, "testserver.pem.key");
export const PUBLICSERVER_CERT_PEM_CRT = path.join(CERTIFICATES, "publicserver.pem.crt");
export const PUBLICSERVER_CERT_PEM_PRVTKEY = path.join(CERTIFICATES, "publicserver.pem.key");

export const CLIENT_CERT_PEM_CRT = path.join(CERTIFICATES, "client.pem.crt");
export const CLIENT_CERT_PEM_PRVTKEY = path.join(CERTIFICATES, "client.pem.key");



export const FLATBUFFERS_SCHEMA_PATH = path.join(ROOT, "flatbuffers", "app.fbs");
export const USERSETTINGS_PATH = path.join(ROOT, "usersettings", "go_here","go_here", "usersettings.ts");



//Location of esp idf tools
export const NVS_PARTITION_GEN_TOOL=path.join(IDF_PATH, "components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py");
export const PART_TOOL=path.join(IDF_PATH, "components/partition_table/parttool.py");
export const NVS_TOOL=path.join(IDF_PATH, "components/nvs_flash/nvs_partition_tool/nvs_tool.py");
export const IDF_BUILD_TOOL=path.join(IDF_PATH, "components/nvs_flash/nvs_partition_tool/nvs_tool.py");