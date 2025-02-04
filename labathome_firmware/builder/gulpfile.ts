import * as gulp from "gulp";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import * as cert from "@klaus-liebler/espidf-vite/certificates";
import * as P from "@klaus-liebler/espidf-vite/paths";
import * as tts from "@klaus-liebler/espidf-vite/text_to_speech";
import * as idf from "@klaus-liebler/espidf-vite/espidf";
import { getLastCommit } from "@klaus-liebler/espidf-vite/git";
import { flatbuffers_generate_c, flatbuffers_generate_ts } from "@klaus-liebler/espidf-vite/flatbuffers";
import { createApiKey } from "@klaus-liebler/espidf-vite/google_cloud";
import {Context, ContextConfig} from "@klaus-liebler/espidf-vite/context"
import { prepare_labathome_files } from "@klaus-liebler/espidf-vite/labathome";
import * as usersettings from "@klaus-liebler/espidf-vite/usersettings_builder"
import {mac_12char, mac_6char, writeFileCreateDirLazy } from "@klaus-liebler/espidf-vite/utils";
import * as vite_helper from "@klaus-liebler/espidf-vite/vite_helper";
import { strInterpolator } from "@klaus-liebler/commons";
import * as usersettings_def from "./symlink_usersettings";

//Default Board Type

export const DEFAULT_BOARD_NAME="LABATHOME"
export const DEFAULT_BOARD_VERSION=150200


//Paths
export const IDF_PATH=globalThis.process.env.IDF_PATH as string;
export const USERPROFILE =globalThis.process.env.USERPROFILE as string;

//Config
const IDF_PROJECT_ROOT = "C:/repos/labathome/labathome_firmware";
const IDF_COMPONENT_WEBMANAGER_ROOT = "C:/repos/espidf-component-webmanager";
const FLATBUFFER_OBJECT_DEFINITIONS_NPM_PROJECT = "C:/repos/npm-packages/@klaus-liebler/flatbuffer-object-definitions"

const SENSACT_COMPONENT_GENERATED_PATH = "C:/repos/sensact/espidf-components/generated";
const BOARDS_BASE_DIR= path.join(USERPROFILE, "netcase/esp32_boards");
const CERTIFICATES = path.join(USERPROFILE, "netcase/certificates");
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

const contextConfig = new ContextConfig("c:/repos/generated", IDF_PROJECT_ROOT, BOARDS_BASE_DIR, DEFAULT_BOARD_NAME, DEFAULT_BOARD_VERSION);

export const doOnce = gulp.series(
  createRootCA,
  createVariousTestCertificates
);


export const buildForCurrent = gulp.series(
  deleteGenerated,
  prepare_board_specific_files,
  compileAndDistributeFlatbuffers,
  generateUsersettings,
  buildAndCompressWebProject,
  createBoardCertificatesLazily,
  createRandomFlashEncryptionKeyLazily,
  createBoardSoundsLazily,
  createCppConfigurationHeader,
  copyMostRecentlyConnectedBoardFilesToCurrent,
  buildFirmware,
  encryptFirmware,
)

export default gulp.series(
  addOrUpdateConnectedBoard,
  buildForCurrent,
  flashEncryptedFirmware,
)
async function deleteGenerated(cb: gulp.TaskFunctionCallback) {
  const c = await Context.get(contextConfig);
  fs.rmSync(c.c.generatedDirectory, {recursive:true, force:true})
}

async function createRandomFlashEncryptionKeyLazily(cb: gulp.TaskFunctionCallback) {
  await idf.createRandomFlashEncryptionKeyLazily(await Context.get(contextConfig));
  cb();
}

async function generateUsersettings(cb: gulp.TaskFunctionCallback) {
  const c=await Context.get(contextConfig)
  await usersettings.generate_usersettings(c, usersettings_def.Build());
  cb();
}

async function buildFirmware(cb: gulp.TaskFunctionCallback) {
  const c=await Context.get(contextConfig)
  const p = new P.Paths(c)
  await idf.buildFirmware(c);
  //await idf.nvs_partition_gen_encrypt(c, ()=>true)

  cb();
} 

async function encryptFirmware(cb: gulp.TaskFunctionCallback){
  await idf.encryptPartitions_Bootloader_App_PartitionTable_OtaData(await Context.get(contextConfig));
  cb();
}

async function flashEncryptedFirmware(cb: gulp.TaskFunctionCallback){
  await idf.flashEncryptedFirmware(await Context.get(contextConfig), false, false);
  cb();
}

export async function addOrUpdateConnectedBoard(cb: gulp.TaskFunctionCallback){
  await Context.get(contextConfig, true);
  return cb();
}

export async function prepare_board_specific_files(cb: gulp.TaskFunctionCallback){
  const c = await Context.get(contextConfig);
  prepare_labathome_files(c);
  //prepare_sensact_files(c);
  return cb()
}

export async function createRootCA(cb: gulp.TaskFunctionCallback) {
  const p = new P.Paths(await Context.get(contextConfig));
  let CA = cert.CreateRootCA(ROOT_CA_COMMON_NAME);
  if (fs.existsSync(P.ROOT_CA_PEM_CRT_FILE) || fs.existsSync(P.ROOT_CA_PEM_CRT_FILE)) {
    return cb(new Error("rootCA Certificate and Key have already been created. Delete them manually to be able to recreate them"));
  }
  writeFileCreateDirLazy(path.join(CERTIFICATES, P.ROOT_CA_PEM_CRT_FILE), CA.certificate);
  writeFileCreateDirLazy(path.join(CERTIFICATES, P.ROOT_CA_PEM_PRVTKEY_FILE), CA.privateKey, cb);
}

export async function createVariousTestCertificates(cb: gulp.TaskFunctionCallback) {
  const p = new P.Paths(await Context.get(contextConfig));
  const this_pc_name = os.hostname();
  let testserverCert = cert.CreateAndSignCert("Testserver", this_pc_name, P.ROOT_CA_PEM_CRT_FILE, P.ROOT_CA_PEM_PRVTKEY_FILE);
  writeFileCreateDirLazy(path.join(CERTIFICATES, P.TESTSERVER_CERT_PEM_CRT_FILE), testserverCert.certificate);
  writeFileCreateDirLazy(path.join(CERTIFICATES, P.TESTSERVER_CERT_PEM_PRVTKEY_FILE), testserverCert.privateKey);

  let publicServerCert = cert.CreateAndSignCert("Klaus Lieber Personal Server", PUBLIC_SERVER_FQDN, P.ROOT_CA_PEM_CRT_FILE, P.ROOT_CA_PEM_PRVTKEY_FILE);
  writeFileCreateDirLazy(path.join(CERTIFICATES, P.PUBLICSERVER_CERT_PEM_CRT_FILE), publicServerCert.certificate);
  writeFileCreateDirLazy(path.join(CERTIFICATES, P.PUBLICSERVER_CERT_PEM_PRVTKEY_FILE), publicServerCert.privateKey);

  let clientCert = cert.CreateAndSignClientCert("labathome_123456", P.ROOT_CA_PEM_CRT_FILE, P.ROOT_CA_PEM_PRVTKEY_FILE);
  writeFileCreateDirLazy(path.join(CERTIFICATES, P.CLIENT_CERT_PEM_CRT_FILE), clientCert.certificate);
  writeFileCreateDirLazy(path.join(CERTIFICATES, P.CLIENT_CERT_PEM_PRVTKEY_FILE), clientCert.privateKey, cb);
}

export async function createGoogleApiKey(cb: gulp.TaskFunctionCallback) {
  await createApiKey()
  cb();
}

export async function compileAndDistributeFlatbuffers(cb: gulp.TaskFunctionCallback) {
  const c= await Context.get(contextConfig)
  const pa = new P.Paths(c);
  await flatbuffers_generate_c(path.join(IDF_COMPONENT_WEBMANAGER_ROOT, "flatbuffers"), pa.GENERATED_FLATBUFFERS_CPP);
  await flatbuffers_generate_ts(path.join(IDF_COMPONENT_WEBMANAGER_ROOT, "flatbuffers"), c.c.generatedDirectory, "flatbuffers_ts");
  cb();
}



export async function createBoardCertificatesLazily(cb: gulp.TaskFunctionCallback) {
  const c=await Context.get(contextConfig);
  const p = new P.Paths(c);
  if (p.existsBoardSpecificPath(P.CERTIFICATES_SUBDIR, P.ESP32_CERT_PEM_PRVTKEY_FILE)
    && p.existsBoardSpecificPath(P.CERTIFICATES_SUBDIR, P.ESP32_CERT_PEM_CRT_FILE)) {
    return cb();
  }
  const hostname = strInterpolator(HOSTNAME_TEMPLATE, {mac_6char:mac_6char(c.b.mac), mac_12char:mac_12char(c.b.mac)});
  let esp32Cert = cert.CreateAndSignCert(hostname, hostname, path.join(CERTIFICATES, P.ROOT_CA_PEM_CRT_FILE), path.join(CERTIFICATES, P.ROOT_CA_PEM_PRVTKEY_FILE));
  p.writeBoardSpecificFileCreateDirLazy(P.CERTIFICATES_SUBDIR, P.ESP32_CERT_PEM_PRVTKEY_FILE, esp32Cert.privateKey);
  p.writeBoardSpecificFileCreateDirLazy(P.CERTIFICATES_SUBDIR, P.ESP32_CERT_PEM_CRT_FILE, esp32Cert.certificate, cb);
}

export async function createBoardSoundsLazily(cb: gulp.TaskFunctionCallback) {
  const c = await Context.get(contextConfig);
  const p = new P.Paths(c);
  await tts.convertTextToSpeech(COMMON_SENTENCES_DE, p.SOUNDS_DE);
  await tts.convertTextToSpeech(APPLICATION_SPECIFIC_SOUNDS, p.boardSpecificPath(P.SOUNDS_DE_SUBDIR));
  cb();
}

async function createObjectWithDefines(c:Context) {
  var defines: any = {};
  for (const [k, v] of Object.entries(c.b.board_settings?.web ?? {})) {
    defines[k] = JSON.stringify(v);
  }

  for (const [k, v] of Object.entries(c.b.board_settings?.firmware ?? {})) {
    defines[k] = JSON.stringify(v);
  }

  defines.__BOARD_NAME__ = JSON.stringify(c.b.board_name);
  defines.__BOARD_VERSION__ = JSON.stringify(c.b.board_version);
  defines.__BOARD_MAC__ = JSON.stringify(c.b.mac);
  defines.__APP_NAME__ = JSON.stringify(APPLICATION_NAME);
  defines.__APP_VERSION__ = JSON.stringify(APPLICATION_VERSION);
  defines.__CREATION_DT__ = JSON.stringify(Math.floor(Date.now() / 1000));
  defines.__GIT_SHORT_HASH__ = JSON.stringify((await getLastCommit(true)).shortHash);
  return defines;
}

export async function buildAndCompressWebProject(cb: gulp.TaskFunctionCallback) {
  const c = await Context.get(contextConfig);
  const pa = new P.Paths(c);
  await vite_helper.buildAndCompressWebProject(path.join(c.c.idfProjectDirectory, "web"), pa.GENERATED_WEB,  await createObjectWithDefines(c));
  return cb();
}


export async function createCppConfigurationHeader(cb: gulp.TaskFunctionCallback) {
  const c=await Context.get(contextConfig);
  const p = new P.Paths(c);
  const defines = await createObjectWithDefines(c);
  var s = "#pragma once\n";
  for (const [k, v] of Object.entries(defines)) {
    s += `#define ${k} ${v}\n`
  }
  p.writeBoardSpecificFileCreateDirLazy("cpp", "__build_config.hh", s);
  return cb();
}

export async function copyMostRecentlyConnectedBoardFilesToCurrent(cb: gulp.TaskFunctionCallback) {
  const c=await Context.get(contextConfig);
  const p = new P.Paths(c);
  fs.cpSync(p.boardSpecificPath(), p.CURRENT_BOARD, { recursive: true });
  return cb();
}




