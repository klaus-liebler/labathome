import { html } from "lit-html"
import "./style/chatbot.css"
import "./style/control_loop_experiment.css"

import { AppController } from "@klaus-liebler/web-components"
import { DefaultScreenController } from "@klaus-liebler/web-components/typescript/screen_controller/screen_controller"
import { SystemController } from "@klaus-liebler/web-components/typescript/screen_controller/systeminfo_controller"
import { WifimanagerController } from "@klaus-liebler/web-components/typescript/screen_controller/wifimanager_controller"
import * as CFG from "@generated/runtimeconfig_ts"
import * as CONST from "@klaus-liebler/web-components/typescript/utils/constants";
import { DevelopCFCController } from "./screen_controller/develop_cfc_controller"
import { HeaterExperimentController } from "./screen_controller/heater_experiment_controller"

// Usersettings (Einstellungs-Screen) und Google-Chatbot sind im C#-Builder-Umbau vorerst entfallen
// (kein Codegenerator fuer usersettings mehr; Chatbot: AppController bekommt optional einen IChatbot).

let app: AppController;
document.addEventListener("DOMContentLoaded", (_e) => {
  app = new AppController("Lab@Home WebUI", CONST.WS_URL, null, `:: Board ${CFG.BOARD_NAME} created at  ${CFG.CREATION_DT_STR} `, null);
  app.AddScreenController("dashboard", new RegExp("^/$"), html`<span>&#127760;</span><span>Home</span>`, new DefaultScreenController(app))
  app.AddScreenController("fbd", new RegExp("^/fbd$"), html`<span>🥽</span><span>Function Block</span>`, new DevelopCFCController(app))
  app.AddScreenController("heater", new RegExp("^/heater$"), html`<span>🥽</span><span>Control Heater</span>`, new HeaterExperimentController(app))
  app.AddScreenController("system", new RegExp("^/system$"), html`<span>🧰</span><span>System Settings</span>`, new SystemController(app))
  app.AddScreenController("wifiman", new RegExp("^/wifiman$"), html`<span>📶</span><span>Wifi Manager</span>`, new WifimanagerController(app))
  app.Startup();
});
