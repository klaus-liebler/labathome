import * as gulp from "gulp";
import fs from "node:fs";
import os from "node:os";
import proc from "node:child_process";

import * as cert from "./gulpfile_helpers/certificates"
import { writeFileCreateDirLazy, writeBoardSpecificFileCreateDirLazy, existsBoardSpecificPath, strInterpolator, boardSpecificPath, createBoardSpecificPathLazy } from "./gulpfile_helpers/utils";
import { PUBLIC_SERVER_FQDN } from "./gulpfile_config";
import * as P from "./paths";
import * as vite from 'vite'
import { createSpeech } from "./gulpfile_helpers/text_to_speech";
import * as idf from "./gulpfile_helpers/espidf";
import * as zlib from "node:zlib"
import { getLastCommit } from "./gulpfile_helpers/git";
import { flatbuffers_generate_c, flatbuffers_generate_ts } from "./gulpfile_helpers/flatbuffers";
import { createApiKey } from "./gulpfile_helpers/google_cloud";
import path from "path";
import {Context} from "./gulpfile_helpers/context"

export const doOnce = gulp.series(
  createRootCA,
  createVariousTestCertificates
);


export const buildForCurrent = gulp.series(
  compileAndDistributeFlatbuffers,
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


export async function addOrUpdateConnectedBoard(cb: gulp.TaskFunctionCallback){
  await Context.get(true);
  return cb();
}


export function createRootCA(cb: gulp.TaskFunctionCallback) {
  let CA = cert.CreateRootCA();
  if (fs.existsSync(P.ROOT_CA_PEM_CRT) || fs.existsSync(P.ROOT_CA_PEM_CRT)) {
    return cb(new Error("rootCA Certificate and Key have already been created. Delete them manually to be able to recreate them"));
  }
  writeFileCreateDirLazy(P.ROOT_CA_PEM_CRT, CA.certificate);
  writeFileCreateDirLazy(P.ROOT_CA_PEM_PRVTKEY, CA.privateKey, cb);
}

export function createVariousTestCertificates(cb: gulp.TaskFunctionCallback) {
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

export async function createGoogleApiKey(cb: gulp.TaskFunctionCallback) {
  await createApiKey()
  cb();
}

export async function compileAndDistributeFlatbuffers(cb: gulp.TaskFunctionCallback) {
  await flatbuffers_generate_c();
  await flatbuffers_generate_ts();
  fs.cpSync(P.GENERATED_FLATBUFFERS_TS, P.DEST_FLATBUFFERS_TYPESCRIPT_WEBUI, { recursive: true });
  fs.cpSync(P.GENERATED_FLATBUFFERS_TS, P.DEST_FLATBUFFERS_TYPESCRIPT_SERVER, { recursive: true });
  cb();
}


export async function createBoardCertificatesLazily(cb: gulp.TaskFunctionCallback) {
  const c=await Context.get();
  if (existsBoardSpecificPath(c, P.CERTIFICATES_SUBDIR, P.ESP32_CERT_PEM_PRVTKEY_FILENAME)
    && existsBoardSpecificPath(c, P.CERTIFICATES_SUBDIR, P.ESP32_CERT_PEM_CRT_FILENAME)) {
    return cb();
  }
  const hostname = strInterpolator(c.a.hostname_template, c.b, c.a);
  let esp32Cert = cert.CreateAndSignCert(hostname, hostname, P.ROOT_CA_PEM_CRT, P.ROOT_CA_PEM_PRVTKEY);
  writeBoardSpecificFileCreateDirLazy(c, P.CERTIFICATES_SUBDIR, P.ESP32_CERT_PEM_PRVTKEY_FILENAME, esp32Cert.privateKey);
  writeBoardSpecificFileCreateDirLazy(c, P.CERTIFICATES_SUBDIR, P.ESP32_CERT_PEM_CRT_FILENAME, esp32Cert.certificate, cb);
}

export async function createBoardSoundsLazily(cb: gulp.TaskFunctionCallback) {
  const c=await Context.get();
  await createSpeech(c);
  cb();
}

async function createObjectWithDefines(c:Context) {
  var defines: any = {};
  for (const [k, v] of Object.entries(c.b.board_settings?.web ?? {})) {
    defines[k] = JSON.stringify(v);
  }
  for (const [k, v] of Object.entries(c.b.board_type_settings?.web ?? {})) {
    defines[k] = JSON.stringify(v);
  }
  for (const [k, v] of Object.entries(c.a.app_settings?.web ?? {})) {
    defines[k] = JSON.stringify(v);
  }
  for (const [k, v] of Object.entries(c.b.board_settings?.firmware ?? {})) {
    defines[k] = JSON.stringify(v);
  }
  for (const [k, v] of Object.entries(c.b.board_type_settings?.firmware ?? {})) {
    defines[k] = JSON.stringify(v);
  }
  for (const [k, v] of Object.entries(c.a.app_settings?.firmware ?? {})) {
    defines[k] = JSON.stringify(v);
  }
  defines.__BOARD_NAME__ = JSON.stringify(c.b.board_name);
  defines.__BOARD_VERSION__ = JSON.stringify(c.b.board_version);
  defines.__BOARD_MAC__ = JSON.stringify(c.b.mac);
  defines.__APP_NAME__ = JSON.stringify(c.a.name);
  defines.__APP_VERSION__ = JSON.stringify(c.a.version);
  defines.__CREATION_DT__ = JSON.stringify(Math.floor(Date.now() / 1000));
  defines.__GIT_SHORT_HASH__ = JSON.stringify((await getLastCommit(true)).shortHash);
  return defines;
}

export async function buildAndCompressWebProject(cb: gulp.TaskFunctionCallback) {
  const c=await Context.get();
  await vite.build({

    root: "../web",
    define: await createObjectWithDefines(c),
    esbuild: {
      //drop:["console", 'debugger'],
      legalComments: 'none',

    },
    build: {
      //minify: true,
      cssCodeSplit: false,
      outDir: boardSpecificPath(c, "web"),
      emptyOutDir: true
    }
  });
  const origPath = boardSpecificPath(c, "web", "index.html")
  const compressedPath = boardSpecificPath(c, "web", "index.compressed.br")
  const result = zlib.brotliCompressSync(fs.readFileSync(origPath));
  fs.writeFileSync(compressedPath, result);
  console.log(`Compressed file written to ${compressedPath}. FileSize = ${result.byteLength} byte = ${(result.byteLength / 1024.0).toFixed(2)} kiB`);
  return cb();
}


export async function createCppConfigurationHeader(cb: gulp.TaskFunctionCallback) {
  const c=await Context.get();
  const defines = await createObjectWithDefines(c);
  var s = "#pragma once\n";
  for (const [k, v] of Object.entries(defines)) {
    s += `#define ${k} ${v}\n`
  }
  writeBoardSpecificFileCreateDirLazy(c, "cpp", "__build_config.hh", s);
  return cb();
}

export async function copyMostRecentlyConnectedBoardFilesToCurrent(cb: gulp.TaskFunctionCallback) {
  const c=await Context.get();
  fs.cpSync(boardSpecificPath(c), "../currentBoardFiles", { recursive: true });
  return cb();
}

export async function createRandomFlashEncryptionKeyLazily(cb: gulp.TaskFunctionCallback) {
  const c=await Context.get();
  if (existsBoardSpecificPath(c, P.FLASH_KEY_SUBDIR, P.FLASH_KEY_FILENAME)) {
    console.info(`flash_encryption key for board  ${c.b.board_name} ${c.b.board_version} with mac 0x${c.b.mac_6char} has already been created`);
    return cb();
  }
  createBoardSpecificPathLazy(c, P.FLASH_KEY_SUBDIR);
  idf.espsecure(`generate_flash_encryption_key --keylen 256 ${boardSpecificPath(c, P.FLASH_KEY_SUBDIR, P.FLASH_KEY_FILENAME)}`, true);
  console.log('Random Flash Encryption Key successfully generated');
  cb();
}

export async function burnFlashEncryptionKeyToAndActivateEncryptedFlash(cb: gulp.TaskFunctionCallback) {
  const c=await Context.get();
  if(c.b.encryption_key_set){
    console.info(`flash_encryption key for board  ${c.b.board_name} ${c.b.board_version} with mac 0x${c.b.mac_12char} has already been burned to efuse`);
    return cb();
  }
   
  idf.espefuse(`--port ${c.b.last_connected_com_port} --do-not-confirm burn_key BLOCK_KEY0 ${boardSpecificPath(c,  P.FLASH_KEY_SUBDIR, P.FLASH_KEY_FILENAME)} XTS_AES_128_KEY`);
  idf.espefuse(`--port ${c.b.last_connected_com_port} --do-not-confirm burn_efuse SPI_BOOT_CRYPT_CNT 1`);
  console.log('Random Flash Encryption Key successfully burned to EFUSE; encryption of flash activated!');
  cb();
}

export async function buildFirmware(cb: gulp.TaskFunctionCallback) {
  const c=await Context.get();
  idf.exec(`idf.py build`, c.a.espIdfProjectDirectory, true);
  console.log('Build-Prozess abgeschlossen!');
  cb();
}
/*
With flash encryption enabled, the following types of data are encrypted by default:
Second Stage Bootloader (Firmware Bootloader)
Partition Table
NVS Key Partition
Otadata
All app type partitions
*/

export async function encryptFirmware(cb: gulp.TaskFunctionCallback) {
  const c=await Context.get();
  var basePath=path.join(c.a.espIdfProjectDirectory, "build");
  [c.f!.bootloader, c.f!.app, c.f!["partition-table"], c.f!.otadata, c.f!.otadata].forEach(s=>{
    idf.espsecure(`encrypt_flash_data --aes_xts --keyfile ${boardSpecificPath(c, "flash_encryption", "key.bin")} --address ${s.offset} --output ${path.join(basePath, s.file.replace(".bin", "-enc.bin"))} ${path.join(basePath, s.file)}`, false);
  })
  console.log('Encryption finished');
  cb();
}



export async function flashEncryptedFirmware(cb: gulp.TaskFunctionCallback) {
  const c=await Context.get();
  var basePath=path.join(c.a.espIdfProjectDirectory, "build");
  [c.f!.bootloader, c.f!.app, c.f!["partition-table"], c.f!.otadata, c.f!.otadata].forEach(s=>{
      const cmd=`--port ${c.b.last_connected_com_port} write_flash --flash_size keep ${s.offset} ${path.join(basePath, s.file.replace(".bin", "-enc.bin"))}`;
      idf.esptool(cmd, false)
    })
  
  
  console.log('Flash finished');
  cb();
}

export async function flashFirmware(cb: gulp.TaskFunctionCallback) {
  const c=await Context.get();
  idf.exec(`idf.py -p ${c.b.last_connected_com_port} flash`, c.a.espIdfProjectDirectory);
  console.log('Flash-Prozess abgeschlossen!');
  cb();
}




export function show_nvs(cb: gulp.TaskFunctionCallback) {
  proc.exec(`py "${P.NVS_TOOL}" --port COM23`, (err, stdout, stderr) => {
    console.log(stdout);
    cb(err);
  });
}


