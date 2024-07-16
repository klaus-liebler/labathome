import { IDF_PATH } from "./gulpfile_config";



import path from "node:path";



export const ROOT = "..";

//top level directories
export const WEB = path.join(ROOT, "web");
export const CERTIFICATES = path.join(ROOT, "certificates");
export const FLATBUFFERS = path.join(ROOT, "flatbuffers");
export const GENERATED = path.join(ROOT, "generated");
export const TESTSERVER = path.join(ROOT, "testserver");

export const ROOT_CA_PEM_CRT = path.join(CERTIFICATES, "rootCA.pem.crt");
export const ROOT_CA_PEM_PRVTKEY = path.join(CERTIFICATES, "rootCA.pem.key");
export const ESP32_CERT_PEM_CRT = path.join(CERTIFICATES, "esp32.pem.crt");
export const ESP32_CERT_PEM_PRVTKEY = path.join(CERTIFICATES, "esp32.pem.key");
export const TESTSERVER_CERT_PEM_CRT = path.join(CERTIFICATES, "testserver.pem.crt");
export const TESTSERVER_CERT_PEM_PRVTKEY = path.join(CERTIFICATES, "testserver.pem.key");
export const PUBLICSERVER_CERT_PEM_CRT = path.join(CERTIFICATES, "publicserver.pem.crt");
export const PUBLICSERVER_CERT_PEM_PRVTKEY = path.join(CERTIFICATES, "publicserver.pem.key");

export const CLIENT_CERT_PEM_CRT = path.join(CERTIFICATES, "client.pem.crt");
export const CLIENT_CERT_PEM_PRVTKEY = path.join(CERTIFICATES, "client.pem.key");



export const FLATBUFFERS_SCHEMA_PATH = path.join(ROOT, "flatbuffers", "app.fbs");
export const USERSETTINGS_PATH = path.join(ROOT, "usersettings", "go_here","go_here", "usersettings.ts");

//Template
export const TEMPLATE_SEND_COMMAND_IMPLEMENTATION = path.join(WEB, "templates", "sensact",  "sendCommandImplementation.template.ts");
export const TEMPLATE_SENSACT_APPS =                path.join(WEB, "templates", "sensact",  "sensactapps.template.ts");

//intermediate and distribution
export const GENERATED_SENSACT_FBS = path.join(GENERATED, "sensact_fbs");
export const GENERATED_SENSACT_TS = path.join(GENERATED, "sensact_ts");
export const GENERATED_FLATBUFFERS_TS = path.join(GENERATED, "flatbuffers_ts");
export const GENERATED_FLATBUFFERS_CPP = path.join(GENERATED, "flatbuffers_cpp");
export const GENERATED_USERSETTINGS = path.join(GENERATED, "usersettings");

export const WEB_SRC_GENERATED = path.join(WEB, "generated")
export const TESTSERVER_GENERATED = path.join(TESTSERVER, "generated")
export const DEST_FLATBUFFERS_TYPESCRIPT_WEBUI = path.join(WEB_SRC_GENERATED, "flatbuffers");
export const DEST_SENSACT_TYPESCRIPT_WEBUI =     path.join(WEB_SRC_GENERATED, "sensact");
export const DEST_USERSETTINGS_PATH =            path.join(WEB_SRC_GENERATED, "usersettings", "usersettings.ts");
export const DEST_FLATBUFFERS_TYPESCRIPT_SERVER = path.join(TESTSERVER_GENERATED, "flatbuffers");

export const HOSTNAME_FILE = path.join(CERTIFICATES, "esp32_hostname.txt");


//Location of esp idf tools
export const NVS_PARTITION_GEN_TOOL=path.join(IDF_PATH, "components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py");
export const PART_TOOL=path.join(IDF_PATH, "components/partition_table/parttool.py");
export const NVS_TOOL=path.join(IDF_PATH, "components/nvs_flash/nvs_partition_tool/nvs_tool.py");