import path from "path";
import { EscapeToVariableName } from "../../usersettings/typescript/utils/usersettings_base";
import { StringBuilderImpl, writeFileCreateDirLazy } from "./utils";
import { GENERATED_USERSETTINGS, USERSETTINGS_PATH, DEST_USERSETTINGS_PATH} from "../paths";
import UserSettings from "../../usersettings/go_here/go_here/usersettings"
import fs from "node:fs";
import { partition_gen } from "./espidf";

function generate_partition_csv() {
  const theusersettings = UserSettings.Build();
  console.log(`User settings has ${theusersettings.length} groups`);
  var codeBuilder = new StringBuilderImpl("key,type,encoding,value");

  theusersettings.forEach((cg, i, a) => {
    codeBuilder.AppendLine(`${cg.Key},namespace,,`);
    cg.items.forEach((ci, j, cia) => {
      ci.RenderNvsPartitionGenerator(codeBuilder);
    });
  });
  writeFileCreateDirLazy(path.join(GENERATED_USERSETTINGS, "usersettings_partition.csv"), codeBuilder.Code);
}
function generate_cpp_accessor() {
  const theusersettings = UserSettings.Build();
  var codeBuilder = new StringBuilderImpl();
  theusersettings.forEach((cg, i, a) => {
    cg.items.forEach((ci, j, cia) => {
      codeBuilder.AppendLine(`constexpr const char ${EscapeToVariableName(cg.Key)}_${EscapeToVariableName(ci.Key)}_KEY[]="${ci.Key}";`)
    });
    cg.RenderCPPConfig(codeBuilder);
    cg.items.forEach((ci, j, cia) => {
      ci.RenderCPPConfig(codeBuilder, cg);
    });
    codeBuilder.AppendLine("}};");
  });
  codeBuilder.AppendLine(`constexpr std::array<const GroupCfg*, ${theusersettings.length}> groups = {`);
  theusersettings.forEach((cg, i, a) => {
    codeBuilder.AppendLine(`\t&${EscapeToVariableName(cg.Key)},`);
  });
  codeBuilder.AppendLine(`};`)
  codeBuilder.AppendLine(``);
  codeBuilder.AppendLine(`namespace settings{`);
  theusersettings.forEach((cg, i, a) => {
    cg.items.forEach((ci, j, cia) => {
      ci.RenderCPPAccessor(codeBuilder, cg);
    });

  });
  codeBuilder.AppendLine(`}`)
  writeFileCreateDirLazy(path.join(GENERATED_USERSETTINGS, "usersettings_config.hh.inc"), codeBuilder.Code);
}


//this is necessary to copy the usersettings (the project specific file, that contains all settings) in the context of the browser client project. 
// There, the usersettings_base.ts is totally different from the one used in the build process
function copy_ts() {
  fs.cpSync(USERSETTINGS_PATH, DEST_USERSETTINGS_PATH, { recursive: true });
}

export function generate_usersettings() {
  generate_partition_csv();
  //compile partition csv to binary partition file
  partition_gen(path.join(GENERATED_USERSETTINGS, "usersettings_partition.csv"), path.join(GENERATED_USERSETTINGS, "usersettings_partition.bin"), false); 
  generate_cpp_accessor();
  copy_ts();
}