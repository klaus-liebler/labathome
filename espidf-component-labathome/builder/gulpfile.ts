import * as gulp from "gulp";
import fs from "node:fs";
import os from "node:os";
import proc from "node:child_process";
import * as esp from "./gulpfile_helpers/esp32"
import { parsePartitions } from "./gulpfile_helpers/partition_parser"
import * as cert from "./gulpfile_helpers/certificates"
import { IBoardInfo, writeFileCreateDirLazy, X02, writeBoardSpecificFileCreateDirLazy, IApplicationInfo, existsBoardSpecificPath, strInterpolator, boardSpecificPath } from "./gulpfile_helpers/gulpfile_utils";
import { CLIENT_CERT_USER_NAME, DEFAULT_BOARD_TYPE_ID, PUBLIC_SERVER_FQDN } from "./gulpfile_config";
import * as P from "./paths";
const { DatabaseSync } = require('node:sqlite');
import * as vite from 'vite'
import path from "node:path";
import { createSpeech } from "./gulpfile_helpers/text_to_speech";
import * as idf from "./gulpfile_helpers/espidf";
import * as util from "node:util"
import * as zlib from "node:zlib"
import { getLastCommit } from "./gulpfile_helpers/git_infos";
import { flatbuffers_generate_c, flatbuffers_generate_ts } from "./gulpfile_helpers/flatbuffers";
import { ApiKeysClient } from '@google-cloud/apikeys'

declare interface IStatementSync {
  all(namedParameters?: any, ...anonymousParameters: Array<null | number | bigint | string | Buffer | Uint8Array>): Array<any>;
  expandedSQL: string;
  get(namedParameters?: any, ...anonymousParameters: Array<null | number | bigint | string | Buffer | Uint8Array>): any | undefined;
  run(namedParameters: any, ...anonymousParameters: Array<null | number | bigint | string | Buffer | Uint8Array>): { changes: number | bigint, lastInsertRowId: number | bigint };
  setAllowBareNamedParameters(enabled: boolean): void;
  setReadBigInts(enabled: boolean): void;
  sourceSQL: string;
}
declare class IDatabaseSync {
  constructor(file: string);
  close(): void;
  open(): void;
  prepare(sql: string): IStatementSync;
}

export function rootCA(cb: gulp.TaskFunctionCallback) {
  let CA = cert.CreateRootCA();
  if (fs.existsSync(P.ROOT_CA_PEM_CRT) || fs.existsSync(P.ROOT_CA_PEM_CRT)) {
    return cb(new Error("rootCA Certificate and Key have already been created. Delete them manually to be able to recreate them"));
  }
  writeFileCreateDirLazy(P.ROOT_CA_PEM_CRT, CA.certificate);
  writeFileCreateDirLazy(P.ROOT_CA_PEM_PRVTKEY, CA.privateKey, cb);
}

export function certificates(cb: gulp.TaskFunctionCallback) {
  const this_pc_name = os.hostname();
  let testserverCert = cert.CreateAndSignCert("Testserver", this_pc_name, P.ROOT_CA_PEM_CRT, P.ROOT_CA_PEM_PRVTKEY);
  writeFileCreateDirLazy(P.TESTSERVER_CERT_PEM_CRT, testserverCert.certificate);
  writeFileCreateDirLazy(P.TESTSERVER_CERT_PEM_PRVTKEY, testserverCert.privateKey);

  let publicServerCert = cert.CreateAndSignCert("Klaus Lieber Personal Server", PUBLIC_SERVER_FQDN, P.ROOT_CA_PEM_CRT, P.ROOT_CA_PEM_PRVTKEY);
  writeFileCreateDirLazy(P.PUBLICSERVER_CERT_PEM_CRT, publicServerCert.certificate);
  writeFileCreateDirLazy(P.PUBLICSERVER_CERT_PEM_PRVTKEY, publicServerCert.privateKey, cb);

  let clientCert = cert.CreateAndSignClientCert("labathome_123456", P.ROOT_CA_PEM_CRT, P.ROOT_CA_PEM_PRVTKEY);
  writeFileCreateDirLazy(P.CLIENT_CERT_PEM_CRT, clientCert.certificate);
  writeFileCreateDirLazy(P.CLIENT_CERT_PEM_PRVTKEY, clientCert.privateKey, cb);
}

var bi: IBoardInfo;

export const buildForCurrent = gulp.series(
  getMostRecentlyConnectedBoardInfo,
  compileAndDistributeFlatbuffers,
  buildWebProject,
  brotliCompress,
  createBoardCertificatesLazily,
  createBoardSoundsLazily,
  createCppConfigurationHeader,
  copyMostRecentlyConnectedBoardFilesToCurrent,
  buildFirmware
)


export default gulp.series(
  addOrUpdateConnectedBoard,
  buildForCurrent,
  flashFirmware,
)

export async function getGitInfo(cb: gulp.TaskFunctionCallback) {
  console.log(await getLastCommit());
}



//Damit das folgende funktioniert, ist das folgende erforderlich: https://cloud.google.com/docs/authentication/provide-credentials-adc
//TL;TR; gclound CLI installieren, in PowerShell: gcloud auth application-default login

export async function createGoogleApiKey(cb: gulp.TaskFunctionCallback) {
  const project_id = 'labathome-434220'
  const parent = `projects/${project_id}/locations/global`

  //It can only contain lowercase letters, numeric characters, and hyphens. It must start with a letter and cannot have a trailing hyphen. The maximum length is "63" characters
  const keyId= "labathome6550c0";//strInterpolator(bi.hostname_template, bi);


  // Instantiates a client
  const apikeysClient = new ApiKeysClient();

  const [operation] = await apikeysClient.createKey({
    parent,
    key:{
      displayName:keyId,
      restrictions:{
        apiTargets:[{service:"generativelanguage.googleapis.com"}]
      },

    },
    keyId
  });
  const [response] = await operation.promise();
  console.log(response);

  const iterable = await apikeysClient.listKeysAsync({ parent });
  for await (const response of iterable) {
    console.log(response.restrictions?.apiTargets![0]);
  }

  cb();
}


export async function compileAndDistributeFlatbuffers(cb: gulp.TaskFunctionCallback) {
  await flatbuffers_generate_c();
  await flatbuffers_generate_ts();
  fs.cpSync(P.GENERATED_FLATBUFFERS_TS, P.DEST_FLATBUFFERS_TYPESCRIPT_WEBUI, { recursive: true });
  fs.cpSync(P.GENERATED_FLATBUFFERS_TS, P.DEST_FLATBUFFERS_TYPESCRIPT_SERVER, { recursive: true });
  cb();
}


export async function addOrUpdateConnectedBoard(cb: gulp.TaskFunctionCallback) {
  var esp32 = await esp.GetESP32Object();
  if (!esp32) {
    throw new Error("No connected board found");
  }
  const db = new DatabaseSync(P.BOARDS_DB) as IDatabaseSync;
  const select_board = db.prepare('SELECT * from boards where mac = (?)');
  var board = select_board.get(esp32.macAsNumber);
  var now = Math.floor(Date.now() / 1000);
  if (!board) {
    const getMcuType = db.prepare('SELECT * from mcu_types where name = (?)');
    var mcuType = getMcuType.get(esp32.chipName)
    if (!mcuType) {
      throw new Error(`Database does not know ${esp32.chipName}`);
    }
    const insert_board = db.prepare('INSERT INTO boards VALUES(?,?,?,?,?,?,?,?)');
    insert_board.run(esp32.macAsNumber, mcuType.id, DEFAULT_BOARD_TYPE_ID, now, now, esp32.comPort.path, null, null);
  } else {
    const update_board = db.prepare('UPDATE boards set last_connected_dt = ?, last_connected_com_port= ? where mac = ? ');
    update_board.run(now, esp32.comPort.path, esp32.macAsNumber);
  }

  return cb();
}

function getMostRecentlyConnectedBoardInfo(cb: gulp.TaskFunctionCallback): void {
  const db = new DatabaseSync(P.BOARDS_DB) as IDatabaseSync;
  const select_board = db.prepare('select b.mac, b.encryption_key_set, m.name as mcu_name, bt.name as board_name, bt.version as board_version, b.first_connected_dt, b.last_connected_dt, b.last_connected_com_port, b.settings as board_settings, bt.settings as board_type_settings from boards as b inner join board_types as bt on bt.id=b.board_type_id inner join mcu_types as m ON m.id=bt.mcu_id ORDER BY last_connected_dt DESC LIMIT 1');
  bi = select_board.get() as IBoardInfo;
  if (!bi) {
    throw new Error("No board found in database");
  }
  bi.board_settings = JSON.parse(bi.board_settings);
  bi.board_type_settings = JSON.parse(bi.board_type_settings);
  bi.mac_12char = X02(bi.mac, 12);
  bi.mac_6char = bi.mac_12char.slice(6);
  const select_app = db.prepare('select a.name, a.version, a.hostname_template, a.settings as app_settings, a.espIdfProjectDirectory from application_types as a inner join app_board_compatibility as c ON a.id=c.application_id WHERE c.board_name=? and c.version_min<=? and c.version_max>=? ORDER BY c.priority DESC LIMIT 1 ');
  var ai = select_app.get(bi.board_name, bi.board_version, bi.board_version) as IApplicationInfo;
  if (!ai) {
    throw new Error("No suitable app found for this board!");
  }
  bi.application_name = ai.name;
  bi.application_version = ai.version;
  bi.application_settings = JSON.parse(ai.app_settings);
  bi.espIdfProjectDirectory = ai.espIdfProjectDirectory;
  bi.hostname_template = ai.hostname_template
  db.close();
  console.log(`Compiling for an ${bi.mcu_name} on board ${bi.board_name} ${bi.board_version} with mac 0x${bi.mac_6char} or ${bi.mac} at port ${bi.last_connected_com_port}`)
  cb();
}


export function createBoardCertificatesLazily(cb: gulp.TaskFunctionCallback) {

  if (existsBoardSpecificPath(bi, P.CERTIFICATES_SUBDIR, P.ESP32_CERT_PEM_PRVTKEY_FILENAME)
    && existsBoardSpecificPath(bi, P.CERTIFICATES_SUBDIR, P.ESP32_CERT_PEM_CRT_FILENAME)) {
    return cb();
  }
  const hostname = strInterpolator(bi.hostname_template, bi);
  let esp32Cert = cert.CreateAndSignCert(hostname, hostname, P.ROOT_CA_PEM_CRT, P.ROOT_CA_PEM_PRVTKEY);
  writeBoardSpecificFileCreateDirLazy(bi, P.CERTIFICATES_SUBDIR, P.ESP32_CERT_PEM_PRVTKEY_FILENAME, esp32Cert.privateKey);
  writeBoardSpecificFileCreateDirLazy(bi, P.CERTIFICATES_SUBDIR, P.ESP32_CERT_PEM_CRT_FILENAME, esp32Cert.certificate, cb);
}

export async function createBoardSoundsLazily(cb: gulp.TaskFunctionCallback) {
  await createSpeech(bi);
  cb();
}

function createObjectWithDefines(bi: IBoardInfo) {
  var defines: any = {};
  for (const [k, v] of Object.entries(bi.board_settings?.web ?? {})) {
    defines[k] = JSON.stringify(v);
  }
  for (const [k, v] of Object.entries(bi.board_type_settings?.web ?? {})) {
    defines[k] = JSON.stringify(v);
  }
  for (const [k, v] of Object.entries(bi.application_settings?.web ?? {})) {
    defines[k] = JSON.stringify(v);
  }
  for (const [k, v] of Object.entries(bi.board_settings?.firmware ?? {})) {
    defines[k] = JSON.stringify(v);
  }
  for (const [k, v] of Object.entries(bi.board_type_settings?.firmware ?? {})) {
    defines[k] = JSON.stringify(v);
  }
  for (const [k, v] of Object.entries(bi.application_settings?.firmware ?? {})) {
    defines[k] = JSON.stringify(v);
  }
  defines.__BOARD_NAME__ = JSON.stringify(bi.board_name);
  defines.__BOARD_VERSION__ = JSON.stringify(bi.board_version);
  defines.__BOARD_MAC__ = JSON.stringify(bi.mac);
  defines.__APP_NAME__ = JSON.stringify(bi.application_name);
  defines.__APP_VERSION__ = JSON.stringify(bi.application_version);
  defines.__CREATION_DT__ = JSON.stringify(Math.floor(Date.now() / 1000));
  return defines;
}

export async function buildWebProject(cb: gulp.TaskFunctionCallback) {
  await vite.build({

    root: "../web",
    define: createObjectWithDefines(bi),
    esbuild: {
      //drop:["console", 'debugger'],
      legalComments: 'none',

    },
    build: {
      //minify: true,
      cssCodeSplit: false,
      outDir: boardSpecificPath(bi, "web"),
      emptyOutDir: true
    }
  });
  cb();
}

export function brotliCompress(cb: gulp.TaskFunctionCallback) {
  const origPath = boardSpecificPath(bi, "web", "index.html")
  const compressedPath = boardSpecificPath(bi, "web", "index.compressed.br")
  zlib.brotliCompress(
    fs.readFileSync(origPath),
    (error: Error | null, result: Buffer) => {
      if (error) {
        throw error;
      }
      fs.writeFile(compressedPath, result, () => {
        console.log(`Compressed file written to ${compressedPath}. FileSize = ${result.byteLength} byte = ${(result.byteLength / 1024.0).toFixed(2)} kiB`);
        cb();
      })
    }
  );
}

export async function createCppConfigurationHeader(cb: gulp.TaskFunctionCallback) {
  var s = "#pragma once\n";
  for (const [k, v] of Object.entries(createObjectWithDefines(bi))) {
    s += `#define ${k} ${v}\n`
  }
  writeBoardSpecificFileCreateDirLazy(bi, "cpp", "__build_config.hh", s, cb);
}

export async function copyMostRecentlyConnectedBoardFilesToCurrent(cb: gulp.TaskFunctionCallback) {
  fs.cp(boardSpecificPath(bi), "./currentBoardFiles", { recursive: true }, cb);
}

export async function createRandomFlashEncryptionKeyLazily(cb: gulp.TaskFunctionCallback) {
  if (!existsBoardSpecificPath(bi, "flash_encryption", "key.bin")) return cb();
  idf.exec(`espsecure.py generate_flash_encryption_key ${boardSpecificPath(bi, "flash_encryption", "key.bin")}`, bi);
  console.log('Random Flash Encryption Key successfully generated');
  cb();
}

export async function burnFlashEncryptionKeyToEfuse(cb: gulp.TaskFunctionCallback) {
  if(bi.encryption_key_set) return cb();
  idf.exec(`espefuse.py --port ${bi.last_connected_com_port} burn_key flash_encryption ${boardSpecificPath(bi, "flash_encryption", "key.bin")}`, bi);
  console.log('Random Flash Encryption Key successfully burnt to EFUSE');
  cb();
}

export async function buildFirmware(cb: gulp.TaskFunctionCallback) {
  idf.exec(`idf.py build`, bi);
  console.log('Build-Prozess abgeschlossen!');
  cb();
}

export async function encryptFirmware(cb: gulp.TaskFunctionCallback) {
  var partitions = parsePartitions(fs.readFileSync("build/partition_table/partition-table.bin"));
  for(const part of partitions){
    console.log(part.toString())
    idf.exec(`espsecure.py encrypt_flash_data --keyfile ${boardSpecificPath(bi, "flash_encryption", "key.bin")} --address 0x${this.offset.toString(16)} --output build_ciphertext.bin build/labathome_firmware.bin`, bi);
  }
  
  console.log('Pre encryption finished-Prozess abgeschlossen!');
  cb();
}


export async function flashEncryptedFirmware(cb: gulp.TaskFunctionCallback) {
  var partitions = parsePartitions(fs.readFileSync("build/partition_table/partition-table.bin"));
  for(const part of partitions){
    console.log(part.toString())
    idf.exec(`idf.py -p ${bi.last_connected_com_port} flash`, bi);
  }
  
  console.log('Pre encryption finished-Prozess abgeschlossen!');
  cb();
}

export async function flashFirmware(cb: gulp.TaskFunctionCallback) {
  idf.exec(`idf.py -p ${bi.last_connected_com_port} flash`, bi);
  console.log('Flash-Prozess abgeschlossen!');
  cb();
}


export async function parsepart(cb: gulp.TaskFunctionCallback) {
  var res = parsePartitions(fs.readFileSync("./partition-table.bin"));
  res.forEach(v => { console.log(v.toString()) })

  cb();
}

export function show_nvs(cb: gulp.TaskFunctionCallback) {
  proc.exec(`py "${P.NVS_TOOL}" --port COM23`, (err, stdout, stderr) => {
    console.log(stdout);
    cb(err);
  });
}


