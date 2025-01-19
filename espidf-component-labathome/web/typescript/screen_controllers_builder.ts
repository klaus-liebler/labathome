import { html } from "lit-html"
import { IAppManagement, IScreenControllerHost } from "./utils/interfaces"
import { DefaultScreenController } from "./screen_controller/screen_controller"
import { DevelopCFCController } from "./screen_controller/develop_cfc_controller"
import { HeaterExperimentController } from "./screen_controller/heater_experiment_controller"
import { SystemController } from "./screen_controller/system_controller"
import { WifimanagerController } from "./screen_controller/wifimanager_controller"

export function BuildScreenControllers(m: IAppManagement, h:IScreenControllerHost): void {
  h.AddScreenController("dashboard", new RegExp("^/$"), html`<span>&#127760;</span><span>Home</span>`, new DefaultScreenController(m))
  h.AddScreenController("fbd", new RegExp("^/fbd$"), html`<span>🥽</span><span>Function Block</span>`, new DevelopCFCController(m))
  h.AddScreenController("heater", new RegExp("^/heater$"), html`<span>🥽</span><span>Control Heater</span>`, new HeaterExperimentController(m))
  h.AddScreenController("system", new RegExp("^/system$"), html`<span>🧰</span><span>System Settings</span>`, new SystemController(m))
  //h.AddScreenController("properties", new RegExp("^/properties$"), html`<span>⌘</span><span>Properties</span>`, UsersettingsController)
  h.AddScreenController("wifiman", new RegExp("^/wifiman$"), html`<span>📶</span><span>Wifi Manager</span>`, new WifimanagerController(m))
}