import * as fs from "node:fs"
import path from "node:path";
import { BOARDS_BASE_DIR } from "../gulpfile_config";
import { Context } from "./context";

export interface IBoardInfo{
  mac:number,
  mac_6char:string,
  mac_12char:string,
  board_name:string,
  board_version:number,
  first_connected_dt:number,
  last_connected_dt:number,
  last_connected_com_port:string,
  mcu_name:string,
  board_settings:any,
  board_type_settings:any,
  encryption_key_set:boolean,
}

export interface IApplicationInfo{
  name:string,
  version:number,
  hostname_template:string,
  app_settings:any,
  espIdfProjectDirectory:string,
}




export function X02(num: number|bigint, len = 2) { let str = num.toString(16); return "0".repeat(len - str.length) + str; }

export function bigint2array(mc:number){
  var ret =new Uint8Array(6);
  ret[5]=Number((mc) & 0xFF)
  ret[4]=Number((mc >> 8) & 0xFF);
  ret[3]=Number((mc >> 16) & 0xFF);
  ret[2]=Number((mc >> 24) & 0xFF);
  ret[1]=Number((mc >> 32) & 0xFF);
  ret[0]=Number((mc >> 40) & 0xFF);
  return ret;
}

export function writeFileCreateDirLazy(file: fs.PathOrFileDescriptor, data: string | NodeJS.ArrayBufferView, callback?: fs.NoParamCallback) {
  fs.mkdirSync(path.dirname(file.toString()), { recursive: true });
  if (callback) {
    fs.writeFile(file, data, callback);
  } else {
    fs.writeFileSync(file, data);
  }
}


export function existsBoardSpecificPath(c:Context, subdir:string, filename:string){
  return fs.existsSync(boardSpecificPath(c, subdir, filename));
}

export function boardSpecificPath(c:Context, subdir?:string, filename?:string){
  if(!subdir)
    return path.join(BOARDS_BASE_DIR, c.b.mac+"_"+c.b.mac_12char);
  else if(!filename)
    return path.join(BOARDS_BASE_DIR, c.b.mac+"_"+c.b.mac_12char, subdir);
  else
    return path.join(BOARDS_BASE_DIR, c.b.mac+"_"+c.b.mac_12char, subdir, filename);
}

export function createBoardSpecificPathLazy(c:Context, subdir:string) {
  var directory= boardSpecificPath(c, subdir);
  fs.mkdirSync(directory, { recursive: true });
}

export function writeBoardSpecificFileCreateDirLazy(c:Context, subdir:string, filename:string, data: string | NodeJS.ArrayBufferView, callback?: fs.NoParamCallback) {
  createBoardSpecificPathLazy(c, subdir)
  if (callback) {
    fs.writeFile(boardSpecificPath(c, subdir, filename), data, callback);
  } else {
    fs.writeFileSync(boardSpecificPath(c, subdir, filename), data);
  }
}


export function createWriteStreamCreateDirLazy(pathLike: fs.PathLike): fs.WriteStream {
  fs.mkdirSync(path.dirname(pathLike.toString()), { recursive: true });
  return fs.createWriteStream(pathLike);
}

export function strInterpolator(str, ...values:any[]) {
  const values_flat=values.flat(1);
  return str.replace(/\${(.*?)}/g, (_match, p1) => {
    if (p1.includes(".")) {
      const keyPath = p1.split(".");
      let currentObj = values_flat;
      for (let i = 0; i < keyPath.length; i++) {
        const key = keyPath[i];
        if (currentObj[key] !== void 0 && typeof currentObj[key] === "object") {
          currentObj = currentObj[key];
        } else {
          return currentObj[key] !== void 0 ? currentObj[key] : "";
        }
      }
    } else {
      return values_flat[p1] !== void 0 ? values_flat[p1] : "";
    }
  });
}