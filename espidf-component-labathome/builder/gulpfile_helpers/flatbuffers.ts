import proc from "node:child_process";
import * as util from "node:util"
import path from "node:path";
import * as fs from "node:fs/promises"
import * as P from "../paths";

export async function flatbuffers_generate(options: string, outputBaseDir: string) {
  const execPromise = util.promisify(proc.exec);
  for (const file of (await fs.readdir(P.FLATBUFFERS_SCHEMA_PATH)).filter(e=>e.endsWith(".fbs"))) {
    console.info(`Processing flatbuffer schema ${file} with options ${options}`)
    const { stdout, stderr } = await execPromise(`flatc ${options} -o ${outputBaseDir} ${path.join(P.FLATBUFFERS_SCHEMA_PATH, file)}`, {
      env: process.env
    });
    console.info(stdout)
    if (stderr) {
      console.error(`Fehlerausgabe: ${stderr}`);
    }
  }
}

export async function flatbuffers_generate_c() {
  await flatbuffers_generate("-c --gen-all", P.GENERATED_FLATBUFFERS_CPP);
}

export async function flatbuffers_generate_ts() {
  await flatbuffers_generate("-T --gen-all --ts-no-import-ext", P.GENERATED_FLATBUFFERS_TS);
}