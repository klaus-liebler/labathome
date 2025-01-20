import { CopyBoardSpecificFiles } from "./utils";
import path from "node:path";
import * as P from "../paths";


import { Context } from "./context";

export function prepare_labathome_files(c:Context) {
  if(!c.a.name.toLocaleLowerCase().startsWith("labathome")){
    console.info("This function should only be called for labathome applications");
  }
  CopyBoardSpecificFiles(P.GENERATED_BOARD_SPECIFIC, [path.join(c.a.espIdfProjectDirectory, "main", "hal")], [c.b.board_version.toString().slice(0,2)]);
}

