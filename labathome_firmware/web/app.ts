import { html } from "lit-html"
import "./style/chatbot.css"
import "./style/control_loop_experiment.css"

import * as c from "@klaus-liebler/web-components"
import * as usersettings from "@generated/usersettings"
import * as CFG from "@generated/runtimeconfig_ts"
import * as CONST from "@klaus-liebler/web-components/typescript/utils/constants";

//create a secrets.ts file next to this file and export a string with your personal GOOGLE API KEY and then do:
//import { GOOGLE_API_KEY } from "./secrets"
//or to deactivate 
//const GOOGLE_API_KEY=""
const GOOGLE_API_KEY=""

let app: c.AppController;
document.addEventListener("DOMContentLoaded", (e) => {
  app = new c.AppController("Lab@Home WebUI", CONST.WS_URL, GOOGLE_API_KEY, true, `:: Board ${CFG.BOARD_NAME} created at  ${CFG.CREATION_DT_STR} `);
  app.AddScreenController("dashboard", new RegExp("^/$"), html`<span>&#127760;</span><span>Home</span>`, new c.DefaultScreenController(app))
  app.AddScreenController("fbd", new RegExp("^/fbd$"), html`<span>🥽</span><span>Function Block</span>`, new c.DevelopCFCController(app))
  app.AddScreenController("heater", new RegExp("^/heater$"), html`<span>🥽</span><span>Control Heater</span>`, new c.HeaterExperimentController(app))
  app.AddScreenController("system", new RegExp("^/system$"), html`<span>🧰</span><span>System Settings</span>`, new c.SystemController(app))
  app.AddScreenController("settings", new RegExp("^/settings$"), html`<span>⌘</span><span>Settings</span>`, new c.UsersettingsController(app, usersettings.Build(CFG.BOARD_NAME, CFG.BOARD_VERSION, [])))
  app.AddScreenController("wifiman", new RegExp("^/wifiman$"), html`<span>📶</span><span>Wifi Manager</span>`, new c.WifimanagerController(app))
  app.Startup();
});


