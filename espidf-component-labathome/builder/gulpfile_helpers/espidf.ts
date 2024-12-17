import proc from "node:child_process";
import path from "path";
import * as util from "node:util"
import { existsBoardSpecificPath, boardSpecificPath, IBoardInfo } from "./gulpfile_utils";


export async function exec(command:string, bi: IBoardInfo){
    console.log(`Executing  ${command}`);
    const execPromise = util.promisify(proc.exec);
    const { stdout, stderr } = await execPromise(`${path.join(globalThis.process.env.IDF_PATH!, "export.bat")} && ${command}`, {
      cwd: bi.espIdfProjectDirectory,
      env: process.env
    });
    if (stderr) {
      throw new Error(`Fehlerausgabe: ${stderr}`);
    }
}