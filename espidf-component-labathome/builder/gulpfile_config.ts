import path from "node:path";

//Default Board Type
const LABATHOME_150100 = 2
const LABATHOME_150200 = 3
export const DEFAULT_BOARD_TYPE_ID=LABATHOME_150100

//Paths
export const IDF_PATH=globalThis.process.env.IDF_PATH as string;
export const USERPROFILE =globalThis.process.env.USERPROFILE as string;
export const PROJECT_ROOT = "..";
export const SENSACT_COMPONENT_GENERATED_PATH = "C:/repos/sensact/espidf-components/generated";
export const BOARDS_BASE_DIR= path.join(USERPROFILE, "netcase/esp32_boards");

//User settings partition
export const USERSETTINGS_PARTITION_NAME="nvs"
export const USERSETTINGS_PARTITION_SIZE_KILOBYTES=16;

//Location of esp idf tools
export const NVS_PARTITION_GEN_TOOL=path.join(IDF_PATH, "components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py");
export const NVS_TOOL=path.join(IDF_PATH, "components/nvs_flash/nvs_partition_tool/nvs_tool.py");
export const PART_TOOL=path.join(IDF_PATH, "components/partition_table/parttool.py");

//Certificate generation
export const DEFAULT_COUNTRY = 'DE';
export const DEFAULT_STATE = 'NRW';
export const DEFAULT_LOCALITY = 'Greven';
export const DEFAULT_ORGANIZATION = 'Klaus Liebler personal';

//not needed export const ROOT_CA_SUBJECT_NAME ="Klaus Liebler"
export const ROOT_CA_COMMON_NAME ="AAA Klaus Liebler personal Root CA"

export const PUBLIC_SERVER_FQDN = "liebler.iui.hs-osnabrueck.de"
