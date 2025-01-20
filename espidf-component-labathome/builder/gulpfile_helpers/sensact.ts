import { writeFileCreateDirLazy } from "./utils";
import path from "node:path";
import fs from "node:fs";
import * as P from "../paths";
import { SENSACT_COMPONENT_GENERATED_PATH } from "../gulpfile_config";
import  {DatabaseSync} from 'node:sqlite';
import { Context } from "./context";

export function prepare_sensact_files(c:Context) {
  const db = new DatabaseSync(P.BOARDS_DB);
  const select_board = db.prepare('select node_id from mac_2_sensact_settings where mac = (?)');
  var node_id_entry = select_board.get(c.b.mac);
  
  const node_id=node_id_entry?node_id_entry["node_id"]:null;
  if (!node_id) {
    console.warn(`No sensact node_id found in database for mac 0x${c.b.mac_6char}. Prepare empty/dummy files for successful build`);
  }

  //jede Node kennt alle ApplicationIds und CommandTypes - das ist also nicht board-spezifisch, deshalb aus dem "common"-Ordner kopieren
  if(!node_id){
    fs.cpSync(path.join(P.FLATBUFFERS_SCHEMA_PATH, "applicationIds.fbs.inc.empty"), path.join(P.GENERATED_SENSACT_FBS, "applicationIds.fbs.inc"), { recursive: true });
    fs.cpSync(path.join(P.FLATBUFFERS_SCHEMA_PATH, "commandTypes.fbs.inc.empty"), path.join(P.GENERATED_SENSACT_FBS, "commandTypes.fbs.inc"), { recursive: true });
  }else{
    fs.cpSync(path.join(SENSACT_COMPONENT_GENERATED_PATH, "common", "applicationIds.fbs.inc"), path.join(P.GENERATED_SENSACT_FBS, "applicationIds.fbs.inc"), { recursive: true });
    fs.cpSync(path.join(SENSACT_COMPONENT_GENERATED_PATH, "common", "commandTypes.fbs.inc"), path.join(P.GENERATED_SENSACT_FBS, "commandTypes.fbs.inc"), { recursive: true });
  }

  //Damit die Applicationen ihre commands versenden können, werden hier passende Funktionen zum Versenden erzeugt
  var content = fs.readFileSync(P.TEMPLATE_SEND_COMMAND_IMPLEMENTATION).toString();
  if(node_id){
    content = content.replace("//TEMPLATE_HERE", fs.readFileSync(path.join(SENSACT_COMPONENT_GENERATED_PATH, "common", "sendCommandImplementation.ts.inc")).toString());
  }
  writeFileCreateDirLazy(path.join(P.GENERATED_SENSACT_TS, "sendCommandImplementation.ts"), content);
  writeFileCreateDirLazy(path.join(P.DEST_SENSACT_TYPESCRIPT_WEBUI, "sendCommandImplementation_copied_during_build.ts"), content);
  writeFileCreateDirLazy(path.join(P.DEST_SENSACT_TYPESCRIPT_SERVER, "sendCommandImplementation_copied_during_build.ts"), content);

  //Alle im Sensact-System bekannten Apps erhalten mit diesem Code einen digitalen Zwilling in der Web-UI
  var content = fs.readFileSync(P.TEMPLATE_SENSACT_APPS).toString();
  if(node_id){
    content = content.replace("//TEMPLATE_HERE", fs.readFileSync(path.join(SENSACT_COMPONENT_GENERATED_PATH, "common", "sensactapps.ts.inc")).toString());
  }
  writeFileCreateDirLazy(path.join(P.GENERATED_SENSACT_TS, "sensactapps.ts"), content);
  writeFileCreateDirLazy(path.join(P.DEST_SENSACT_TYPESCRIPT_WEBUI, "sensactapps_copied_during_build.ts"), content);
  writeFileCreateDirLazy(path.join(P.DEST_SENSACT_TYPESCRIPT_SERVER, "sensactapps_copied_during_build.ts"), content);
}

/*
Wir brauchen noch:
- die richige HAL-Implementierung
- die richtige Busmaster-Konfiguration (also insbesondere welche I2C-PortExtender angeschlossen sind und wie die konfiguriert sind)
  -->Auslagern in eine Funktion, die einen Pointer auf std::vector<AbstractBusmaster*> zurückgibt
- die richtige host-Konfiguration, also welche Hosts laufen sollen
   --> Auslagern in eine Funktion, die einen Pointer auf std::vector<sensact::iHost*> zurückgibt
*/
