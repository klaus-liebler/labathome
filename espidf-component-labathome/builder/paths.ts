import { BOARDS_BASE_DIR, PROJECT_ROOT } from "./gulpfile_config";
import path from "node:path";



//Dies hier sind ausschließlich relative Pfade bezogen auf die in gulpfile_config.ts definierten Pfade


//top level directories
export const WEB = path.join(PROJECT_ROOT, "web");

//Board Specific
export const BOARDS_DB = path.join(BOARDS_BASE_DIR, "builder.db")
export const SOUNDS_DE_SUBDIR = "sounds_de";
export const CERTIFICATES_SUBDIR = "certificates";
export const ESP32_CERT_PEM_CRT_FILENAME = "esp32.pem.crt";
export const ESP32_CERT_PEM_PRVTKEY_FILENAME = "esp32.pem.key";
export const ESP32_CERT_PEM_PUBKEY_FILENAME = "esp32.pem.pubkey";
export const FLASH_KEY_SUBDIR ="flash_encryption"
export const FLASH_KEY_FILENAME= "key.bin"

//various servers
export const TESTSERVER = path.join(PROJECT_ROOT, "testserver");
export const CERTIFICATES = path.join(PROJECT_ROOT, "certificates");
export const ROOT_CA_PEM_CRT = path.join(CERTIFICATES, "rootCA.pem.crt");
export const ROOT_CA_PEM_PRVTKEY = path.join(CERTIFICATES, "rootCA.pem.key");
export const TESTSERVER_CERT_PEM_CRT = path.join(CERTIFICATES, "testserver.pem.crt");
export const TESTSERVER_CERT_PEM_PRVTKEY = path.join(CERTIFICATES, "testserver.pem.key");
export const PUBLICSERVER_CERT_PEM_CRT = path.join(CERTIFICATES, "publicserver.pem.crt");
export const PUBLICSERVER_CERT_PEM_PRVTKEY = path.join(CERTIFICATES, "publicserver.pem.key");

export const CLIENT_CERT_PEM_CRT = path.join(CERTIFICATES, "client.pem.crt");
export const CLIENT_CERT_PEM_PRVTKEY = path.join(CERTIFICATES, "client.pem.key");


//Special sources
export const FLATBUFFERS_SCHEMA_PATH = path.join(PROJECT_ROOT, "flatbuffers");
export const USERSETTINGS_PATH = path.join(PROJECT_ROOT, "usersettings", "go_here","go_here", "usersettings.ts");

//intermediate and distribution
export const GENERATED = path.join(PROJECT_ROOT, "generated");
export const GENERATED_SENSACT_FBS = path.join(GENERATED, "sensact_fbs");
export const GENERATED_SENSACT_TS = path.join(GENERATED, "sensact_ts");
export const GENERATED_FLATBUFFERS_TS = path.join(GENERATED, "flatbuffers_ts");
export const GENERATED_FLATBUFFERS_CPP = path.join(GENERATED, "flatbuffers_cpp");
export const GENERATED_USERSETTINGS = path.join(GENERATED, "usersettings");
export const GENERATED_BOARD_SPECIFIC = path.join(GENERATED, "board_specific_cpp");

export const WEB_SRC_GENERATED = path.join(WEB, "generated")
export const TESTSERVER_GENERATED = path.join(TESTSERVER, "generated")
export const DEST_FLATBUFFERS_TYPESCRIPT_WEBUI = path.join(WEB_SRC_GENERATED, "flatbuffers");
export const DEST_SENSACT_TYPESCRIPT_WEBUI =     path.join(WEB_SRC_GENERATED, "sensact");
export const DEST_USERSETTINGS_PATH =            path.join(WEB_SRC_GENERATED, "usersettings", "usersettings.ts");

//Template
export const TEMPLATE_SEND_COMMAND_IMPLEMENTATION = path.join(WEB, "templates", "sensact",  "sendCommandImplementation.template.ts");
export const TEMPLATE_SENSACT_APPS =                path.join(WEB, "templates", "sensact",  "sensactapps.template.ts");


export const DEST_FLATBUFFERS_TYPESCRIPT_SERVER = path.join(TESTSERVER_GENERATED, "flatbuffers");
export const DEST_SENSACT_TYPESCRIPT_SERVER =     path.join(TESTSERVER_GENERATED, "sensact");



