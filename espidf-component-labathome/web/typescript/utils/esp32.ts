export const chip2name = new Map<number, string>([[1, "ESP32"], [2, "ESP32-S2"], [9, "ESP32-S3"], [5, "ESP32-C3"], [6, "ESP32-H2"], [12, "ESP32-C2"]]);
export const chipfeature = new Map<number, string>([[0, "Embedded Flash Memory"], [1, "2.4GHz WiFi"], [4, "Bluetooth LE"], [5, "Bluetooth Classic"], [6, "IEEE 802.15.4"], [7, "Embedded Psram"]]);/* Chip feature flags, used in esp_chip_info_t */


const partitionstate = new Map<number, string>([
  [0, "ESP_OTA_IMG_NEW"],
  [1, "ESP_OTA_IMG_PENDING_VERIFY"],
  [2, "ESP_OTA_IMG_VALID"],
  [3, "ESP_OTA_IMG_INVALID"],
  [4, "ESP_OTA_IMG_ABORTED"],
  [0xFFFFFFFF, "ESP_OTA_IMG_UNDEFINED"]
]);
const partitionsubtypesapp = new Map<number, string>([
  [0x00, "ESP_PARTITION_SUBTYPE_APP_FACTORY"],
  [0x10, "ESP_PARTITION_SUBTYPE_APP_OTA_0"],// = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 0,  //!< OTA partition 0
  [0x11, "ESP_PARTITION_SUBTYPE_APP_OTA_1"],// = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 1,  //!< OTA partition 1
  [0x12, "ESP_PARTITION_SUBTYPE_APP_OTA_2"],// = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 2,  //!< OTA partition 2
  [0x13, "ESP_PARTITION_SUBTYPE_APP_OTA_3"],// = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 3,  //!< OTA partition 3
  [0x14, "ESP_PARTITION_SUBTYPE_APP_OTA_4"],// = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 4,  //!< OTA partition 4
  [0x15, "ESP_PARTITION_SUBTYPE_APP_OTA_5"],// = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 5,  //!< OTA partition 5
  [0x16, "ESP_PARTITION_SUBTYPE_APP_OTA_6"],// = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 6,  //!< OTA partition 6
  [0x17, "ESP_PARTITION_SUBTYPE_APP_OTA_7"],// = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 7,  //!< OTA partition 7
  [0x18, "ESP_PARTITION_SUBTYPE_APP_OTA_8"],// = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 8,  //!< OTA partition 8
  [0x19, "ESP_PARTITION_SUBTYPE_APP_OTA_9"],// = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 9,  //!< OTA partition 9
  [0x1A, "ESP_PARTITION_SUBTYPE_APP_OTA_10"],// = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 10,//!< OTA partition 10
  [0x1B, "ESP_PARTITION_SUBTYPE_APP_OTA_11"],// = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 11,//!< OTA partition 11
  [0x1C, "ESP_PARTITION_SUBTYPE_APP_OTA_12"],// = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 12,//!< OTA partition 12
  [0x1D, "ESP_PARTITION_SUBTYPE_APP_OTA_13"],// = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 13,//!< OTA partition 13
  [0x1E, "ESP_PARTITION_SUBTYPE_APP_OTA_14"],// = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 14,//!< OTA partition 14
  [0x1F, "ESP_PARTITION_SUBTYPE_APP_OTA_15"],// = ESP_PARTITION_SUBTYPE_APP_OTA_MIN + 15,//!< OTA partition 15
  [0x20, "ESP_PARTITION_SUBTYPE_APP_TEST"],
]);
const partitionsubtypesdata = new Map<number, string>([
  [0x00, "ESP_PARTITION_SUBTYPE_DATA_OTA"],// = 0x00,                                    //!< OTA selection partition
  [0x01, "ESP_PARTITION_SUBTYPE_DATA_PHY"],// = 0x01,                                    //!< PHY init data partition
  [0x02, "ESP_PARTITION_SUBTYPE_DATA_NVS"],// = 0x02,                                    //!< NVS partition
  [0x03, "ESP_PARTITION_SUBTYPE_DATA_COREDUMP"],// = 0x03,                               //!< COREDUMP partition
  [0x04, "ESP_PARTITION_SUBTYPE_DATA_NVS_KEYS"],// = 0x04,                               //!< Partition for NVS keys
  [0x05, "ESP_PARTITION_SUBTYPE_DATA_EFUSE_EM"],// = 0x05,                               //!< Partition for emulate eFuse bits
  [0x06, "ESP_PARTITION_SUBTYPE_DATA_UNDEFINED"],// = 0x06,                              //!< Undefined (or unspecified) data partition

  [0x80, "ESP_PARTITION_SUBTYPE_DATA_ESPHTTPD"],// = 0x80,                               //!< ESPHTTPD partition
  [0x81, "ESP_PARTITION_SUBTYPE_DATA_FAT"],// = 0x81,                                    //!< FAT partition
  [0x82, "ESP_PARTITION_SUBTYPE_DATA_SPIFFS"],// = 0x82,                         
]);

export function findPartitionState(ota_state:number|undefined): string {
  if(ota_state===undefined){
    return "Unknown Partition State"
  }
  return partitionstate.has(ota_state) ? partitionstate.get(ota_state)! : "Unknown State " + ota_state.toFixed(0);
}

export function partitionString(original: string | null| undefined, def: string) {
  return original?.charAt(0) == '\0xFF' ? def : original
}

export function findPartitionSubtype(type:number|undefined, subtype:number|undefined): string {
  if(type===undefined || subtype==undefined){
    return "Unknown Partition Type"
  }
  switch (type) {
    case 0x00:
      return partitionsubtypesapp.has(subtype) ? partitionsubtypesapp.get(subtype)! : "Unknown App Subtype";
    case 0x01:
      return partitionsubtypesdata.has(subtype) ? partitionsubtypesdata.get(subtype)! : "Unknown Data Subtype";
    default:
      return "Unknown Partition Type";
  }
}


export function findChipModel(chipId: number): string {
  return chip2name.has(chipId) ? chip2name.get(chipId)! : "Unknown Chip";
}

export function findChipFeatures(featuresBitSet: number): string {
  let s = "";
  for (let entry of chipfeature.entries()) {
    let mask = 1 << entry[0];
    if ((featuresBitSet & mask) != 0) {
      s += entry[1] + ", ";
    }
  }
  return s;
}