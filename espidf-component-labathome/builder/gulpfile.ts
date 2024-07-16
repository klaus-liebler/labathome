import * as gulp from "gulp";
import fs from "node:fs";
import os from "node:os";
import proc from "node:child_process";
import {getMac} from "./esp32/esp32"
import {parsePartitions} from "./esp32/partition_parser"

import * as cert from "./certificates"


import {writeFileCreateDirLazy } from "./gulpfile_utils";
import { CLIENT_CERT_USER_NAME, COM_PORT, ESP32_HOSTNAME_TEMPLATE, PUBLIC_SERVER_FQDN } from "./gulpfile_config";
import * as P from "./paths";

import * as fbc from "../web/typescript/flowchart/FlowchartCompiler"

function compileDefaultFbd(cb: gulp.TaskFunctionCallback){
  var compiler:fbc.FlowchartCompiler = new fbc.FlowchartCompiler()
}

function clean(cb: gulp.TaskFunctionCallback) {
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


exports.build = gulp.series(
  clean,
  usersettings_generate_cpp_code,
  usersettings_distribute_ts,
  usersettings_createPartition,
  snsct.fetchGeneratedFlatbufferSources,
  snsct.sendCommandImplementation_template,
  snsct.sensactapps_template,
  flatbuffers_generate_c,
  flatbuffers_generate_ts,
  flatbuffers_distribute_ts,
  );
  
  exports.clean = clean;
  exports.rootCA = (cb: gulp.TaskFunctionCallback) => {
    let CA = cert.CreateRootCA();
    if(fs.existsSync(P.ROOT_CA_PEM_CRT) || fs.existsSync(P.ROOT_CA_PEM_CRT)){
      return cb(new Error("rootCA Certificate and Key have already been created. Delete them manually to be able to recreate them"));
    }
    writeFileCreateDirLazy(P.ROOT_CA_PEM_CRT, CA.certificate);
    writeFileCreateDirLazy(P.ROOT_CA_PEM_PRVTKEY, CA.privateKey, cb);
  }
  
  exports.certificates = (cb: gulp.TaskFunctionCallback) => {
    const hostname = fs.readFileSync(P.HOSTNAME_FILE).toString();//esp32host_2df5c8
    const this_pc_name = os.hostname();
    
    let esp32Cert = cert.CreateAndSignCert("SYSTEM_"+hostname, hostname, P.ROOT_CA_PEM_CRT, P.ROOT_CA_PEM_PRVTKEY);
    writeFileCreateDirLazy(P.ESP32_CERT_PEM_CRT, esp32Cert.certificate);
    writeFileCreateDirLazy(P.ESP32_CERT_PEM_PRVTKEY, esp32Cert.privateKey);
    
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
  
  exports.gethostname = async (cb: gulp.TaskFunctionCallback) => {
    var mac= await getMac(COM_PORT);
    console.log(`The MAC adress is ${mac.toString()}`);
    var hostname = ESP32_HOSTNAME_TEMPLATE(mac);
    console.log(`The Hostname will be ${hostname}`);
    writeFileCreateDirLazy(P.HOSTNAME_FILE, hostname, cb);
  }

  
  exports.parsepart = async (cb: gulp.TaskFunctionCallback) => {
    const partitionfile = fs.readFileSync("./partition-table.bin");
    
    var res= parsePartitions(partitionfile);
    res.forEach(v=>{
      console.log(v.toString());
    })
    cb();
  }

  exports.show_nvs =(cb: gulp.TaskFunctionCallback)=>{
    proc.exec(`py "${P.NVS_TOOL}" --port "${COM_PORT}"`, (err, stdout, stderr) => {
      console.log(stdout);
      cb(err);
    });
  }


  exports.flashusersettings = flashusersettings;
  

  exports.default = exports.build
  