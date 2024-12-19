import { execSync, spawnSync } from 'node:child_process';
import path from "path";
import { IBoardInfo } from "./gulpfile_utils";


export function exec(command:string, bi: IBoardInfo, suppressStdOut:boolean=false){
  const cmd = `${path.join(globalThis.process.env.IDF_PATH!, "export.bat")} && ${command}`
  console.info(`Executing ${cmd}`)
  const stdout = execSync(cmd, {
      cwd: bi.espIdfProjectDirectory,
      env: process.env
    });
    if(!suppressStdOut && stdout)
      console.log(stdout.toString());
}

export function espefuse(params:string, bi: IBoardInfo, suppressStdOut:boolean=false){
  tool("espefuse.py", params, bi, suppressStdOut)
}

export function espsecure(params:string, bi: IBoardInfo, suppressStdOut:boolean=false){
  tool("espsecure.py", params, bi, suppressStdOut)
}

export function esptool(params:string, bi: IBoardInfo, suppressStdOut:boolean=false){
  tool("esptool.py", params, bi, suppressStdOut)
}

export function tool(tool:string, params:string, bi: IBoardInfo, suppressStdOut:boolean=false){
  const cmd = `${path.join(globalThis.process.env.IDF_PATH!, "export.bat")} && python.exe ${path.join(globalThis.process.env.IDF_PATH!, "components", "esptool_py", "esptool", tool)} ${params} `
  console.info(`Executing ${cmd}`)
  const stdout = execSync(cmd, {
      cwd: bi.espIdfProjectDirectory,
      env: process.env
    });
    if(!suppressStdOut && stdout)
      console.log(stdout.toString());
}