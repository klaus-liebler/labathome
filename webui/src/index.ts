import "./index.scss";
import { FlowchartOptions, FlowchartData, Flowchart } from "./flowchart";
declare let Chartist: any;

abstract class ScreenController {
    constructor(protected div: HTMLDivElement) { this.hide(); }
    get ElementId() { return this.div.id }
    abstract startup();
    public show() {
        this.div.style.display = "block";
    }
    public hide() {
        this.div.style.display = "none";
    }
}

class DevelopController extends ScreenController {
    constructor(public div: HTMLDivElement) { super(div); }
    public startup() {
        let data: FlowchartData = {
            operators: [
                {
                    caption: "AND1",
                    type: "AND",
                    id: "AND1",
                    posX: 10,
                    posY: 10,
                },
                {
                    caption: "AND2",
                    type: "AND",
                    id: "AND2",
                    posX: 210,
                    posY: 10,
                },
                {
                    caption: "AND3",
                    type: "AND",
                    id: "AND3",
                    posX: 410,
                    posY: 20,
                }
                ,
                {
                    caption: "ADD1",
                    type: "ADD",
                    id: "ADD1",
                    posX: 10,
                    posY: 200,
                }
                ,
                {
                    caption: "ADD2",
                    type: "ADD",
                    id: "ADD2",
                    posX: 210,
                    posY: 200,
                }
            ],
            links: [
                {
                    color: null,
                    fromId: "AND1",
                    fromOutput: 0,
                    toId: "AND2",
                    toInput: 0
                }
            ]
        };
        let options = new FlowchartOptions();
        options.data = data;
        let fc = new Flowchart(this.div, options);
    }
}

class DashboardController extends ScreenController {
    constructor(public div: HTMLDivElement) {
        super(div);
    }
    public startup() {
    }
}

class ReportsController extends ScreenController {
    constructor(public div: HTMLDivElement) {
        super(div);
    }
    public startup() {
        let data = {
            // A labels array that can contain any sort of values
            labels: [],
            // Our series array that contains series objects or in this case series data arrays
            series: [
                []
            ],
            low: 0,
            high: 40
        };
        let options = {
            width: 600,
            height: 400
        };
        let currVal = 20;

        for (let i = 0; i < 10; i++) {
            data.labels.push(10 - i);
            data.series[0][i] = currVal;
        }
        // Create a new line chart object where as first parameter we pass in a selector
        // that is resolving to our chart container element. The Second parameter
        // is the actual data object.
        let chart = new Chartist.Line('.ct-chart', data, options);
        let timer = window.setInterval(() => {
            let foo = data.series[0].slice(1);
            foo.push(20 + 3 * Math.random())
            data.series[0] = foo;
            chart.update(data, options, false);

        }, 500);
        window.setTimeout(() => { window.clearInterval(timer) }, 10000);
    }
}

class AppController {
    private develop: DevelopController;
    private dashboard: DashboardController;
    private reports: ReportsController;
    private linkmap = new Map<string, ScreenController>();
    private stateDiv: HTMLElement;

    constructor() {
        this.stateDiv = document.getElementById("spnConnectionState");
    }

    private SetApplicationState(state: string) {
        this.stateDiv.innerHTML = state;
    }


    public startup() {
        this.dashboard = new DashboardController(<HTMLDivElement>document.getElementById("screen_dashboard"));
        this.develop = new DevelopController(<HTMLDivElement>document.getElementById("screen_develop"));
        this.reports = new ReportsController(<HTMLDivElement>document.getElementById("screen_reports"));
        this.linkmap.set("showDashboard", this.dashboard);
        this.linkmap.set("showReports", this.reports);
        this.linkmap.set("showDevelop", this.develop);
        document.querySelectorAll("nav a").forEach((a: HTMLAnchorElement) => {
            a.onclick = (e) => {
                for (let controller of this.linkmap.values()) {
                    if ((<HTMLAnchorElement>e.target).id == "show_" + controller.ElementId) {
                        controller.show();
                    }
                    else {
                        controller.hide();
                    }
                }
            }
        });


        this.dashboard.show();
        this.dashboard.startup();
        this.develop.startup();
        this.reports.startup();

        this.SetApplicationState("WebSocket is not connected");
        let websocket = new WebSocket('ws://' + location.hostname + '/w');
        


        websocket.onopen = e => {
            this.SetApplicationState('WebSocket connection opened');
            document.getElementById("test").innerHTML = "WebSocket is connected!";
        }

        websocket.onmessage = (evt) => {
            var msg = evt.data;
            let value: string;
            switch (msg.charAt(0)) {
                case 'L':
                    console.log(msg);
                    value = msg.replace(/[^0-9\.]/g, '');
                    switch (value) {
                        case "0": document.getElementById("led1").style.backgroundColor = "black"; break;
                        case "1": document.getElementById("led1").style.backgroundColor = "green"; break;
                        case "2": document.getElementById("led2").style.backgroundColor = "black"; break;
                        case "3": document.getElementById("led2").style.backgroundColor = "green"; break;
                    }
                    console.log("Led = " + value);
                    break;
                default:
                    let p = JSON.parse(evt.data);
                    document.getElementById("td_myName").innerText = p.d.myName;
                    document.getElementById("td_temperature").innerText = p.d.temperature;
                    document.getElementById("td_humidity").innerText = p.d.humidity;
                    document.getElementById("td_heap").innerText = p.info.heap;
                    document.getElementById("td_time").innerText = p.info.time;
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

        document.querySelectorAll("#pButtons button").forEach((b: HTMLButtonElement) => {
            b.onclick = (e: MouseEvent) => {
                websocket.send("L" + b.dataset.rel);
            };
        });

    }
}

let app: AppController;
document.addEventListener("DOMContentLoaded", (e) => {
    app = new AppController();
    app.startup();
});


