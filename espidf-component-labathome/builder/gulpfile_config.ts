import { X02 } from "./gulpfile_utils";
const LABATHOME_150100 = 2
const LABATHOME_150200 = 3

export const DEFAULT_BOARD_TYPE_ID=LABATHOME_150200

export const USERSETTINGS_PARTITION_NAME="nvs"
export const USERSETTINGS_PARTITION_SIZE_KILOBYTES=16;

export const IDF_PATH=globalThis.process.env.IDF_PATH as string;

export const ESP32_HOSTNAME_TEMPLATE_XX = (mac:Uint8Array)=>{
    return `labathome_${X02(mac[3])}${X02(mac[4])}${X02(mac[5])}`;
}

export const DEFAULT_COUNTRY = 'DE';
export const DEFAULT_STATE = 'NRW';
export const DEFAULT_LOCALITY = 'Greven';
export const DEFAULT_ORGANIZATION = 'Klaus Liebler personal';

//not needed export const ROOT_CA_SUBJECT_NAME ="Klaus Liebler"
export const ROOT_CA_COMMON_NAME ="AAA Klaus Liebler personal Root CA"

export const PUBLIC_SERVER_FQDN = "liebler.iui.hs-osnabrueck.de"

export const CLIENT_CERT_USER_NAME = "mosquitto_user"