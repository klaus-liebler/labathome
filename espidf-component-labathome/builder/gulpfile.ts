import * as gulp from "gulp";
import fs from "node:fs";
import os from "node:os";
import proc from "node:child_process";
import * as esp from "./esp32/esp32"
import { parsePartitions } from "./esp32/partition_parser"
import * as cert from "./certificates"


import {writeFileCreateDirLazy } from "./gulpfile_utils";
import { CLIENT_CERT_USER_NAME, COM_PORT, ESP32_HOSTNAME_TEMPLATE, PUBLIC_SERVER_FQDN } from "./gulpfile_config";
import * as P from "./paths";

const { DatabaseSync } = require('node:sqlite');


export function clean(cb: gulp.TaskFunctionCallback) {
  [P.GENERATED, P.WEB_SRC_GENERATED, P.DEST_FLATBUFFERS_TYPESCRIPT_SERVER].forEach((path) => {
    fs.rmSync(path, { recursive: true, force: true });
  });
  cb();
}

function flatbuffers_generate_c(cb: gulp.TaskFunctionCallback) {
  proc.exec(`flatc -c --gen-all -o ${P.GENERATED_FLATBUFFERS_CPP} ${P.FLATBUFFERS_SCHEMA_PATH}`, (err, stdout, stderr) => {
    stdout
    stderr
    cb(err);
  });
}

function flatbuffers_generate_ts(cb: gulp.TaskFunctionCallback) {
  proc.exec(`flatc -T --gen-all --ts-no-import-ext -o ${P.GENERATED_FLATBUFFERS_TS} ${P.FLATBUFFERS_SCHEMA_PATH}`, (err, stdout, stderr) => {
    stdout
    stderr
    cb(err);
  });
}

function flatbuffers_distribute_ts(cb: gulp.TaskFunctionCallback) {
  fs.cpSync(P.GENERATED_FLATBUFFERS_TS, P.DEST_FLATBUFFERS_TYPESCRIPT_WEBUI, { recursive: true });
  fs.cpSync(P.GENERATED_FLATBUFFERS_TS, P.DEST_FLATBUFFERS_TYPESCRIPT_SERVER, { recursive: true });
  cb();
}


export default gulp.series(
    clean,
    
  );



export function rootCA(cb: gulp.TaskFunctionCallback){
  let CA = cert.CreateRootCA();
  if (fs.existsSync(P.ROOT_CA_PEM_CRT) || fs.existsSync(P.ROOT_CA_PEM_CRT)) {
    return cb(new Error("rootCA Certificate and Key have already been created. Delete them manually to be able to recreate them"));
  }
  writeFileCreateDirLazy(P.ROOT_CA_PEM_CRT, CA.certificate);
  writeFileCreateDirLazy(P.ROOT_CA_PEM_PRVTKEY, CA.privateKey, cb);
}


export function certs(cb: gulp.TaskFunctionCallback){
  const hostname = fs.readFileSync(P.HOSTNAME_FILE).toString();//esp32host_2df5c8
  let esp32Cert = cert.CreateAndSignCert(hostname, hostname, P.ROOT_CA_PEM_CRT, P.ROOT_CA_PEM_PRVTKEY);
  writeFileCreateDirLazy(P.ESP32_CERT_PEM_PRVTKEY, esp32Cert.privateKey);
  writeFileCreateDirLazy(P.ESP32_CERT_PEM_CRT, esp32Cert.certificate, cb);
  
}

//creates certificates with a given public key
export function cert_gk(cb: gulp.TaskFunctionCallback){
  const hostname = fs.readFileSync(P.HOSTNAME_FILE).toString();//esp32host_2df5c8
  var esp32Cert=cert.CreateAndSignCertWithGivenPublicKey(P.ESP32_CERT_PEM_PUBKEY, hostname, hostname, P.ROOT_CA_PEM_CRT, P.ROOT_CA_PEM_PRVTKEY);
  writeFileCreateDirLazy(P.ESP32_CERT_PEM_CRT, esp32Cert, cb);
}

export function certificates_servers(cb: gulp.TaskFunctionCallback){
  const this_pc_name = os.hostname();
  let testserverCert = cert.CreateAndSignCert("Testserver", this_pc_name, P.ROOT_CA_PEM_CRT, P.ROOT_CA_PEM_PRVTKEY);
  writeFileCreateDirLazy(P.TESTSERVER_CERT_PEM_CRT, testserverCert.certificate);
  writeFileCreateDirLazy(P.TESTSERVER_CERT_PEM_PRVTKEY, testserverCert.privateKey);

  let publicServerCert = cert.CreateAndSignCert("Klaus Lieber Personal Server", PUBLIC_SERVER_FQDN, P.ROOT_CA_PEM_CRT, P.ROOT_CA_PEM_PRVTKEY);
  writeFileCreateDirLazy(P.PUBLICSERVER_CERT_PEM_CRT, publicServerCert.certificate);
  writeFileCreateDirLazy(P.PUBLICSERVER_CERT_PEM_PRVTKEY, publicServerCert.privateKey, cb);

  let clientCert = cert.CreateAndSignClientCert(CLIENT_CERT_USER_NAME, P.ROOT_CA_PEM_CRT, P.ROOT_CA_PEM_PRVTKEY);
  writeFileCreateDirLazy(P.CLIENT_CERT_PEM_CRT, clientCert.certificate);
  writeFileCreateDirLazy(P.CLIENT_CERT_PEM_PRVTKEY, clientCert.privateKey, cb);
}


const DEFAULT_BOARD_NAME="LabAtHome"
const DEFAULT_BOARD_SEMANTIC_VERSION=150300
declare interface IStatementSync{
  all(namedParameters:any, ...anonymousParameters:Array<null|number|bigint|string|Buffer|Uint8Array>):Array<any>;
  expandedSQL:string;
  get(namedParameters:any, ...anonymousParameters:Array<null|number|bigint|string|Buffer|Uint8Array>):any|undefined;
  run(namedParameters:any, ...anonymousParameters:Array<null|number|bigint|string|Buffer|Uint8Array>):{changes:number|bigint, lastInsertRowId:number|bigint};
  setAllowBareNamedParameters(enabled:boolean):void;
  setReadBigInts(enabled:boolean):void;
  sourceSQL:string;
}
declare interface IDatabaseSync{
  close():void;
  open():void;
  prepare(sql:string):IStatementSync;
}
export async function updateConnectedBoard(cb: gulp.TaskFunctionCallback) {
  try {
    var esp32 = await esp.GetESP32Object();
    if(!esp32){
      console.error("No connected board found");
      return cb();
    }
    console.log(`The MAC adress is ${esp32.macAsUint8Array.toString()} resp. ${esp32.macAsBigint}`);
    const db = new DatabaseSync("./builder.db") as IDatabaseSync;
    const select_board = db.prepare('SELECT * from boards where mac = (?)');
    var board=select_board.get(esp32.macAsBigint);
    var now=Math.floor(Date.now()/1000);
    if(!board){
      const getMcuType = db.prepare('SELECT * from mcu_types where name = (?)');
      var mcuType= getMcuType.get(esp32.chipName)
      if(!mcuType){
        console.error(`Database does not know ${esp32.chipName}`);
        return cb();
      }
      const insert_board = db.prepare('INSERT INTO boards (mac, mcu, board_name, board_semantic_version, first_connected_dt, last_connected_dt) VALUES(?,?,?,?,?,?)');
      insert_board.run(esp32.macAsBigint, mcuType.id, DEFAULT_BOARD_NAME, DEFAULT_BOARD_SEMANTIC_VERSION, now, now);
    }else{
      const update_board = db.prepare('UPDATE boards set last_connected_dt = ? where mac = ? ');
      update_board.run(now, esp32.macAsBigint);
    }
    //var hostname = ESP32_HOSTNAME_TEMPLATE(mac);
    //console.log(`The Hostname will be ${hostname}`);
    //writeFileCreateDirLazy(P.HOSTNAME_FILE, hostname, cb);
  } catch (error) {
    console.error(`There was an error ${error}`);
  }
  
}


export async function parsepart(cb: gulp.TaskFunctionCallback) {
  const partitionfile = fs.readFileSync("./partition-table.bin");

  var res = parsePartitions(partitionfile);
  res.forEach(v => {
    console.log(v.toString());
  })
  cb();
}

export function show_nvs(cb: gulp.TaskFunctionCallback){
  proc.exec(`py "${P.NVS_TOOL}" --port "${COM_PORT}"`, (err, stdout, stderr) => {
    console.log(stdout);
    cb(err);
  });
}


