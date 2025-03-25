/*
flatc auf path-Variable
esp-idf installieren
*/
import * as gulp from "gulp";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import * as cert from "@klaus-liebler/espidf-vite-secure-build-tools/certificates";
import * as P from "@klaus-liebler/espidf-vite-secure-build-tools/paths";
import * as tts from "@klaus-liebler/espidf-vite-secure-build-tools/text_to_speech";
import * as idf from "@klaus-liebler/espidf-vite-secure-build-tools/espidf";
import { getLastCommit } from "@klaus-liebler/espidf-vite-secure-build-tools/git";
import * as ascii_art from "@klaus-liebler/espidf-vite-secure-build-tools/ascii_art";
import { flatbuffers_generate_c, flatbuffers_generate_ts } from "@klaus-liebler/espidf-vite-secure-build-tools/flatbuffers";
import { createApiKey } from "@klaus-liebler/espidf-vite-secure-build-tools/google_cloud";
import {Context, ContextConfig} from "@klaus-liebler/espidf-vite-secure-build-tools/context"
import * as usersettings from "@klaus-liebler/espidf-vite-secure-build-tools/usersettings_builder"
import {cleanNpmExcept_PackageJson_node_modules, mac_12char, mac_6char, writeFileCreateDirLazy } from "@klaus-liebler/espidf-vite-secure-build-tools/utils";
import * as vite_helper from "@klaus-liebler/espidf-vite-secure-build-tools/vite_helper";
import { MyFavouriteDateTimeFormat, strInterpolator } from "@klaus-liebler/commons";
import * as usersettings_def from "./symlink_usersettings";
import * as cfg from "@klaus-liebler/espidf-vite-secure-build-tools/key_value_file_helper"

//Default Board Type
export const DEFAULT_BOARD_NAME="LABATHOME"
export const DEFAULT_BOARD_VERSION=150200


//Paths
export const IDF_PATH=globalThis.process.env.IDF_PATH as string;
export const USERPROFILE =globalThis.process.env.USERPROFILE as string;

//Config
const FLASH_ENCYRPTION_STRENGTH=idf.EncryptionStrength.AES256
const IDF_PROJECT_ROOT = "C:\\repos\\labathome\\labathome_firmware";
const IDF_COMPONENT_WEBMANAGER_ROOT = "C:/repos/espidf-component-webmanager";
const GENERATED_ROOT = "c:\\repos\\generated";

const BOARDS_BASE_DIR= path.join(USERPROFILE, "netcase/esp32_boards");
const CERTIFICATES = path.join(USERPROFILE, "netcase/certificates");

//Root Certificate Data
//not needed export const ROOT_CA_SUBJECT_NAME ="Klaus Liebler"
const ROOT_CA_COMMON_NAME ="AAA Klaus Liebler personal Root CA"
const PUBLIC_SERVER_FQDN = "liebler.iui.hs-osnabrueck.de"
const HOSTNAME_TEMPLATE = "labathome_${mac_6char}"

const APPLICATION_NAME = "labathome"
const APPLICATION_VERSION = "1.0"

const APPLICATION_SPECIFIC_SOUNDS:Array<tts.FilenameAndSsml> = [
  new tts.FilenameAndSsml("ready", "<speak>Willkommen! <lang xml:lang='en-US'>Lab@Home</lang><say-as interpret-as='characters'>${mac_6char}</say-as> ist bereit</speak>"),
]

const COMMON_SENTENCES_DE:Array<tts.FilenameAndSsml> = [
  new tts.FilenameAndSsml("resistor_hot", "<speak>Achtung! Der Heizwiderstand wird zu heiß!</speak>"),
  new tts.FilenameAndSsml("boring", "<speak>Mir wird langweilig</speak>"),
  new tts.FilenameAndSsml("ok", "<speak>Alles ok</speak>"),
  new tts.FilenameAndSsml("nok", "<speak>Nicht ok</speak>"),
  new tts.FilenameAndSsml("alarm_co2", "<speak>CO2 Alarm</speak>"),
  new tts.FilenameAndSsml("alarm_temperature", "<speak>Temperaturalarm</speak>")
]

const contextConfig = new ContextConfig(GENERATED_ROOT, IDF_PROJECT_ROOT, BOARDS_BASE_DIR, DEFAULT_BOARD_NAME, DEFAULT_BOARD_VERSION);

export const doOnce = gulp.series(
  createRootCA,
  createVariousTestCertificates
);

export const buildForCurrent = gulp.series(
  createFiles,
  buildAndCompressWebProject,
  buildAndEncryptFirmware,
)

export default gulp.series(
  addOrUpdateConnectedBoard,
  buildForCurrent,
  flashEncryptedFirmware,
)

export async function addOrUpdateConnectedBoard(cb: gulp.TaskFunctionCallback){
  return Context.get(contextConfig, true);
}

async function buildAndEncryptFirmware(cb: gulp.TaskFunctionCallback) {
  const c=await Context.get(contextConfig)
  await idf.buildFirmware(c);
  return idf.encryptPartitions_Bootloader_App_PartitionTable_OtaData(c);
} 

async function flashEncryptedFirmware(cb: gulp.TaskFunctionCallback){
  const c = await Context.get(contextConfig)
  await idf.burnFlashEncryptionKeyAndActivateEncryptedFlash(c, FLASH_ENCYRPTION_STRENGTH)
  return idf.flashEncryptedFirmware(c, true, false, true);
}

export async function createRootCA(cb: gulp.TaskFunctionCallback) {
  let CA = cert.CreateRootCA(ROOT_CA_COMMON_NAME);
  if (fs.existsSync(P.ROOT_CA_PEM_CRT_FILE) || fs.existsSync(P.ROOT_CA_PEM_CRT_FILE)) {
    return cb(new Error("rootCA Certificate and Key have already been created. Delete them manually to be able to recreate them"));
  }
  writeFileCreateDirLazy(path.join(CERTIFICATES, P.ROOT_CA_PEM_CRT_FILE), CA.certificate);
  writeFileCreateDirLazy(path.join(CERTIFICATES, P.ROOT_CA_PEM_PRVTKEY_FILE), CA.privateKey);
  cb();
}

export async function createVariousTestCertificates(cb: gulp.TaskFunctionCallback) {
  const this_pc_name = os.hostname();
  let testserverCert = cert.CreateAndSignCert("Testserver", "127.0.0.1", ["localhost", this_pc_name], path.join(CERTIFICATES, P.ROOT_CA_PEM_CRT_FILE), path.join(CERTIFICATES, P.ROOT_CA_PEM_PRVTKEY_FILE));
  writeFileCreateDirLazy(path.join(CERTIFICATES, P.TESTSERVER_CERT_PEM_CRT_FILE), testserverCert.certificate);
  writeFileCreateDirLazy(path.join(CERTIFICATES, P.TESTSERVER_CERT_PEM_PRVTKEY_FILE), testserverCert.privateKey);

  let publicServerCert = cert.CreateAndSignCert("Klaus Lieber Personal Server", "127.0.0.1", [PUBLIC_SERVER_FQDN], path.join(CERTIFICATES, P.ROOT_CA_PEM_CRT_FILE), path.join(CERTIFICATES, P.ROOT_CA_PEM_PRVTKEY_FILE));
  writeFileCreateDirLazy(path.join(CERTIFICATES, P.PUBLICSERVER_CERT_PEM_CRT_FILE), publicServerCert.certificate);
  writeFileCreateDirLazy(path.join(CERTIFICATES, P.PUBLICSERVER_CERT_PEM_PRVTKEY_FILE), publicServerCert.privateKey);

  let clientCert = cert.CreateAndSignClientCert("labathome_123456", path.join(CERTIFICATES, P.ROOT_CA_PEM_CRT_FILE), path.join(CERTIFICATES, P.ROOT_CA_PEM_PRVTKEY_FILE));
  writeFileCreateDirLazy(path.join(CERTIFICATES, P.CLIENT_CERT_PEM_CRT_FILE), clientCert.certificate);
  writeFileCreateDirLazy(path.join(CERTIFICATES, P.CLIENT_CERT_PEM_PRVTKEY_FILE), clientCert.privateKey);
  cb();
}

export async function createGoogleApiKey(cb: gulp.TaskFunctionCallback) {
  return createApiKey()
}

export async function createFiles(cb: gulp.TaskFunctionCallback) {
  const c= await Context.get(contextConfig)

  //Flatbuffers
  fs.rmSync(c.p.GENERATED_FLATBUFFERS_CPP, {recursive:true, force:true})
  cleanNpmExcept_PackageJson_node_modules(c.p.GENERATED_FLATBUFFERS_TS);
  await flatbuffers_generate_c([path.join(IDF_COMPONENT_WEBMANAGER_ROOT, "flatbuffers")], c.p.GENERATED_FLATBUFFERS_CPP);
  await flatbuffers_generate_ts([path.join(IDF_COMPONENT_WEBMANAGER_ROOT, "flatbuffers")], c.p.GENERATED_FLATBUFFERS_TS);

  //Certificates (lazy)
  if (!c.p.existsBoardSpecificPath(P.CERTIFICATES_SUBDIR, P.ESP32_CERT_PEM_PRVTKEY_FILE)
    || !c.p.existsBoardSpecificPath(P.CERTIFICATES_SUBDIR, P.ESP32_CERT_PEM_CRT_FILE)) {
    const hostname = strInterpolator(HOSTNAME_TEMPLATE, {mac_6char:mac_6char(c.b.mac), mac_12char:mac_12char(c.b.mac)});
    let esp32Cert = cert.CreateAndSignCert(hostname, "192.168.4.1", [hostname, hostname+".local", hostname+".fritz.box"], path.join(CERTIFICATES, P.ROOT_CA_PEM_CRT_FILE), path.join(CERTIFICATES, P.ROOT_CA_PEM_PRVTKEY_FILE));
    c.p.writeBoardSpecificFileCreateDirLazy(P.CERTIFICATES_SUBDIR, P.ESP32_CERT_PEM_PRVTKEY_FILE, esp32Cert.privateKey);
    c.p.writeBoardSpecificFileCreateDirLazy(P.CERTIFICATES_SUBDIR, P.ESP32_CERT_PEM_CRT_FILE, esp32Cert.certificate);
    }

  //Flash Encryption Key
  idf.createRandomFlashEncryptionKeyLazily(await Context.get(contextConfig), FLASH_ENCYRPTION_STRENGTH);
  
  //User Settings
  await usersettings.generate_usersettings(c, usersettings_def.Build(c.b.board_name, c.b.board_version, []));
   
  //Sounds
  await tts.convertTextToSpeech(COMMON_SENTENCES_DE, c.p.P_SOUNDS_DE);
  await tts.convertTextToSpeech(APPLICATION_SPECIFIC_SOUNDS, c.p.boardSpecificPath(P.SOUNDS_DE_SUBDIR));

  //Config files
  const defs = await createObjectWithDefines(c);
  cfg.createCMakeJsonConfigFile(c, defs);
  cfg.createCppConfigurationHeader(c, defs);
  cfg.createTypeScriptRuntimeConfigProject(c, defs);
  cb();
}

async function createObjectWithDefines(c:Context) {
  var defines: Record<string, string|Array<string>|number> = {};
  
  for (const [k, v] of Object.entries(c.b.board_settings?.web ?? {})) {
    defines[k] = JSON.stringify(v);
  }

  for (const [k, v] of Object.entries(c.b.board_settings?.firmware ?? {})) {
    defines[k] = JSON.stringify(v);
  }
  const now = new Date();
  defines.HOSTNAME = strInterpolator(HOSTNAME_TEMPLATE, {mac_6char:mac_6char(c.b.mac), mac_12char:mac_12char(c.b.mac)});;
  defines.BOARD_NAME = c.b.board_name;
  defines.BOARD_VERSION = c.b.board_version;
  defines.BOARD_ROLES=c.b.board_roles;
  defines.BOARD_MAC = c.b.mac;
  defines.BOARD_DIRECTORY = c.p.boardSpecificPath();
  defines.APP_NAME = APPLICATION_NAME;
  defines.APP_VERSION = APPLICATION_VERSION;
  defines.CREATION_DT = Math.floor(now.valueOf() / 1000);
  defines.CREATION_DT_STR = now.toLocaleString("de-DE", MyFavouriteDateTimeFormat)
  defines.GIT_SHORT_HASH = (await getLastCommit(true)).shortHash;
  defines.BANNER = ascii_art.createAsciiArt(`${APPLICATION_NAME}`);
  return defines;
}

export async function buildAndCompressWebProject(cb: gulp.TaskFunctionCallback) {
  const c = await Context.get(contextConfig);
  await vite_helper.buildAndCompressWebProject(path.join(c.c.idfProjectDirectory, "web"), c.p.GENERATED_WEB);
  cb();
}