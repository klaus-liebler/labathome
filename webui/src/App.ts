import { ExperimentController } from "./ExperimentController";
import { ScreenController, ControllerState } from "./ScreenController";
import { DevelopCFCController } from "./DevelopCFCController";

class DashboardController extends ScreenController {
    public onFirstStart(): void { }
    public onRestart(): void { }
    public onStop(): void { }
    public onCreate() { }
    constructor(public div: HTMLDivElement) {
        super(div);
    }

}

class ReportsController extends ScreenController {
    public onFirstStart(): void { }
    public onRestart(): void { }
    public onStop(): void { }
    constructor(public div: HTMLDivElement) {
        super(div);
    }
    public onCreate() {
        return;

    }
}



class AppController {

    private stateDiv: HTMLDivElement;
    private activeControllerIndex: number;
    private screenControllers: ScreenController[];



    constructor() {
        this.stateDiv = <HTMLDivElement>document.getElementById("spnConnectionState")!;
        this.screenControllers = [];
        this.activeControllerIndex = 0;
    }

    private SetApplicationState(state: string) {
        this.stateDiv.innerHTML = state;

    }

    private setActiveScreen(newIndex: number) {
        this.screenControllers.forEach((controller, i) => {
            if (i == newIndex) {
                controller.showDIV();
                if (controller.State == ControllerState.CREATED) {
                    controller.onFirstStart();
                    controller.State = ControllerState.STARTED;
                }
                else {
                    controller.onRestart();
                    controller.State = ControllerState.STARTED;
                }
            } else {
                controller.hideDIV();
                if (controller.State == ControllerState.STARTED) {
                    controller.onStop();
                    controller.State = ControllerState.STOPPED;
                }
            }
        });
        this.activeControllerIndex = newIndex;
    }

    public startup() {
        this.screenControllers.push(new DashboardController(<HTMLDivElement>document.getElementById("screen_dashboard")));
        this.screenControllers.push(new DevelopCFCController(<HTMLDivElement>document.getElementById("screen_develop")));
        this.screenControllers.push(new ReportsController(<HTMLDivElement>document.getElementById("screen_reports")));
        this.screenControllers.push(new ExperimentController(<HTMLDivElement>document.getElementById("screen_experiment")));
        this.screenControllers.forEach((sc) => sc.onCreate());

        this.setActiveScreen(0);
        let id2index = new Map<string, number>();
        this.screenControllers.forEach((value, index) => { id2index.set("show_" + value.ElementId, index) })
        document.querySelectorAll<HTMLAnchorElement>("nav a").forEach((a: HTMLAnchorElement) => {
            let id = a.id;
            let index = id2index.get(a.id) || 0;
            a.onclick = (e) => this.setActiveScreen(index);
        });



        /*
        this.SetApplicationState("WebSocket is not connected");
        let websocket = new WebSocket('ws://' + location.hostname + '/w');
        websocket.onopen = e => {
            this.SetApplicationState('WebSocket connection opened');
            document.getElementById("test")!.innerHTML = "WebSocket is connected!";
        }
        websocket.onmessage = (evt) => {
            var msg = evt.data;
            let value: string;
            switch (msg.charAt(0)) {
                case 'L':
                    console.log(msg);
                    value = msg.replace(/[^0-9\.]/g, '');
                    switch (value) {
                        case "0": document.getElementById("led1")!.style.backgroundColor = "black"; break;
                        case "1": document.getElementById("led1")!.style.backgroundColor = "green"; break;
                        case "2": document.getElementById("led2")!.style.backgroundColor = "black"; break;
                        case "3": document.getElementById("led2")!.style.backgroundColor = "green"; break;
                    }
                    console.log("Led = " + value);
                    break;
                default:
                    let p = JSON.parse(evt.data);
                    document.getElementById("td_myName")!.innerText = p.d.myName;
                    document.getElementById("td_temperature")!.innerText = p.d.temperature;
                    document.getElementById("td_humidity")!.innerText = p.d.humidity;
                    document.getElementById("td_heap")!.innerText = p.info.heap;
                    document.getElementById("td_time")!.innerText = p.info.time;
                    break;
            }
        }

        websocket.onclose = (e) => {
            console.log('Websocket connection closed due to '+e.reason);
            this.SetApplicationState('Websocket connection closed due to '+e.reason);
        }

        websocket.onerror = (evt) => {
            console.log('Websocket error: ' + evt.returnValue);
            this.SetApplicationState("WebSocket error!" + evt.returnValue);
        }

        document.querySelectorAll<HTMLButtonElement>("#pButtons button").forEach((b: HTMLButtonElement) => {
            b.onclick = (e: MouseEvent) => {
                websocket.send("L" + b.dataset.rel);
            };
        });
*/
    }
}

let app: AppController;
document.addEventListener("DOMContentLoaded", (e) => {
    app = new AppController();
    app.startup();
});


