import { html } from "lit-html"
import "./style/chatbot.css"
import "./style/control_loop_experiment.css"

import { AppController } from "@klaus-liebler/web-components"
import { OverviewScreenController } from "@klaus-liebler/web-components/typescript/screen_controller/overview_controller"
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
  app = new AppController("Lab@Home", CONST.WS_URL, null, `erstellt ${CFG.CREATION_DT_STR}`, null, { hostLabel: CFG.BOARD_NAME });
  // Reihenfolge = Reihenfolge im Arbeitsbereich-Umschalter; der erste ist die Startseite
  app.AddScreenController("dashboard", new RegExp("^/$"), html`<span>Übersicht</span>`, new OverviewScreenController(app, () => app.Workspaces),
    { icon: "dashboard", group: "Übersicht", description: "Alle Arbeitsbereiche" })
  app.AddScreenController("fbd", new RegExp("^/fbd$"), html`<span>Function Block</span>`, new DevelopCFCController(app),
    { title: "Funktionsbaustein", icon: "plc", group: "Experimente", description: "Steuerung als Funktionsbausteinsprache entwickeln" })
  app.AddScreenController("heater", new RegExp("^/heater$"), html`<span>Control Heater</span>`, new HeaterExperimentController(app),
    { title: "Heizungsregelung", icon: "temperature", group: "Experimente", description: "Regelkreis der Heizung untersuchen" })
  app.AddScreenController("system", new RegExp("^/system$"), html`<span>System</span>`, new SystemController(app),
    { icon: "chip", group: "Gerät", description: "Firmware, Dateien, Neustart" })
  app.AddScreenController("wifiman", new RegExp("^/wifiman$"), html`<span>WLAN</span>`, new WifimanagerController(app),
    { icon: "wifi", group: "Gerät", description: "WLAN-Netze suchen und verbinden" })
  app.Startup();
});
