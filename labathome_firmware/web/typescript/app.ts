import { html } from "lit-html";
import "../style/app.css";
import * as c from "@klaus-liebler/web-components"
import * as usersettings from "../../../../generated/nvs_ts"
import { WS_URL, WS_URL_ESP32_AP, WS_URL_ESP32_STA } from "@klaus-liebler/web-components/typescript/utils/constants";

function BuildScreenControllers(m: c.interfaces.IAppManagement, h:c.interfaces.IScreenControllerHost): void {
  h.AddScreenController("dashboard", new RegExp("^/$"), html`<span>&#127760;</span><span>Home</span>`, new c.DefaultScreenController(m))
  h.AddScreenController("fbd", new RegExp("^/fbd$"), html`<span>🥽</span><span>Function Block</span>`, new c.DevelopCFCController(m))
  h.AddScreenController("heater", new RegExp("^/heater$"), html`<span>🥽</span><span>Control Heater</span>`, new c.HeaterExperimentController(m))
  h.AddScreenController("system", new RegExp("^/system$"), html`<span>🧰</span><span>System Settings</span>`, new c.SystemController(m))
  h.AddScreenController("settings", new RegExp("^/settings$"), html`<span>⌘</span><span>Settings</span>`, new c.UsersettingsController(m, usersettings.Build()))
  h.AddScreenController("wifiman", new RegExp("^/wifiman$"), html`<span>📶</span><span>Wifi Manager</span>`, new c.WifimanagerController(m))
}

let app: c.AppController;
document.addEventListener("DOMContentLoaded", (e) => {
  app = new c.AppController();
  BuildScreenControllers(app, app);
  app.Startup(WS_URL, "");
});


