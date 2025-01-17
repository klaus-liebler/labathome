import { $ } from "./utils/common";
import { Chart} from 'chart.js';
import { ScreenController } from "./ScreenController";
import { AppManagement } from "./AppManagement";
import { SerializeContext } from "./flowchart/SerializeContext";

const DE_de = new Intl.NumberFormat('de-DE');
const CHART_EACH_INTERVAL = 2;

export class AirspeedExperimentController extends ScreenController {
    private butRecord: HTMLButtonElement;
    private butStop: HTMLButtonElement;
    private butDelete: HTMLButtonElement;
    private tbody: HTMLTableSectionElement;
    private tfirstRow: HTMLTableRowElement;
    private inputSetpointFan: HTMLInputElement;
    private inputSetpointAirspeed: HTMLInputElement;
    private inputServoCL: HTMLInputElement;
    private inputServoOL: HTMLInputElement;
    private inputKP: HTMLInputElement;
    private inputKI: HTMLInputElement;
    private inputKD: HTMLInputElement;
    private timer: number | undefined;
    private chart: Chart;
    private counter = 10 ^ 6;
    private mode: number = 0;
    private seconds = 0;

    private recording = false;

    public onFirstStart(): void {
        this.timer = window.setInterval(() => { this.sendAndReceive(); }, 1000);
    }
    public onRestart(): void {
        this.timer = window.setInterval(() => { this.sendAndReceive(); }, 1000);
    }
    public onStop(): void {
        window.clearInterval(this.timer);
        this.butStop.hidden = true;
        this.butRecord.hidden = false;
        this.counter = 10 ^ 6;
    }
    public onCreate() {
        this.resetData();

    }

    private resetData() {
        this.chart.data!.labels = [];
        this.chart.data!.datasets!.forEach((dataset) => {
            dataset!.data = [];
        });
        this.chart.update();
        this.tbody.innerText = "";
        this.seconds = 0;
    }


    private onModeChange(newMode: number) {
        switch (newMode) {
            case 0:
                document.querySelectorAll('.airspeedexperiment_closedloopctrl').forEach((v, k) => {
                    (<HTMLElement>v).style.display = "none";
                });
                document.querySelectorAll('.airspeedexperiment_openloopctrl').forEach((v, k) => {
                    (<HTMLElement>v).style.display = "none";
                });
                break;
            case 1:
                document.querySelectorAll('.airspeedexperiment_closedloopctrl').forEach((v, k) => {
                    (<HTMLElement>v).style.display = "none";
                });
                document.querySelectorAll('.airspeedexperiment_openloopctrl').forEach((v, k) => {
                    (<HTMLElement>v).style.display = "inline-block";
                });
                break;
            case 2:
                document.querySelectorAll('.airspeedexperiment_closedloopctrl').forEach((v, k) => {
                    (<HTMLElement>v).style.display = "inline-block";
                });
                document.querySelectorAll('.airspeedexperiment_openloopctrl').forEach((v, k) => {
                    (<HTMLElement>v).style.display = "none";
                });
                break;
        }
        this.mode = newMode;
    }

    private sendAndReceive() {
        let buffer = new ArrayBuffer(256);
        let ctx = new SerializeContext(buffer);
        ctx.writeU32(this.mode);
        if (this.mode == 0) {
            ctx.writeF32(0);
            ctx.writeF32(0);
        } else if (this.mode == 1) {
            ctx.writeF32(this.inputSetpointFan.valueAsNumber);
            ctx.writeF32(this.inputServoOL.valueAsNumber);
        }
        else {
            ctx.writeF32(this.inputSetpointAirspeed.valueAsNumber);
            ctx.writeF32(this.inputServoCL.valueAsNumber);
        }
        ctx.writeF32(this.inputKP.valueAsNumber);
        ctx.writeF32(this.inputKI.valueAsNumber);
        ctx.writeF32(this.inputKD.valueAsNumber);

        let xhr = new XMLHttpRequest;
        xhr.onerror = (e) => { console.log("Fehler beim XMLHttpRequest!"); };
        xhr.open("PUT", "/airspeedexperiment", true);
        xhr.responseType = "arraybuffer";
        xhr.onload = (e) => {
            let SetpointAirspeed: number, Fan: number, Servo: number, ActualAirspeed: number;
            let arrayBuffer = xhr.response; // Note: not oReq.responseText
            if (!arrayBuffer || arrayBuffer.byteLength != 4 + 4 + 4 + 4) {
                console.error("! arrayBuffer || arrayBuffer.byteLength!=4+4+4+4");
                SetpointAirspeed = 0;
                Fan = 0;
                Servo = 0;
                ActualAirspeed = 20 + (-5 + 10 * Math.random());
            }
            else {
                let ctx = new SerializeContext(arrayBuffer);
                SetpointAirspeed = ctx.readF32();
                Fan = ctx.readF32();
                Servo = ctx.readF32();
                ActualAirspeed = ctx.readF32();
            }
            let now = new Date(Date.now());

            if (this.recording) {
                let tr = $.HtmlAsFirstChild(this.tbody, "tr", []);
                for (let i = 0; i < 6; i++) {
                    $.Html(tr, "td", [], [], this.tfirstRow.children[i].textContent!);
                }
                if (this.counter >= CHART_EACH_INTERVAL) {
                    if (this.chart.data!.labels!.length > 100) {
                        this.chart.data!.labels?.shift();
                        this.chart.data!.datasets!.forEach((dataset) => {
                            dataset!.data!.shift();
                        });
                    }
                    this.chart.data!.labels!.push(now.toLocaleTimeString("de-DE"));
                    this.chart.data?.datasets![0].data?.push(SetpointAirspeed);
                    this.chart.data?.datasets![1].data?.push(ActualAirspeed);
                    this.chart.data?.datasets![2].data?.push(Fan);
                    this.chart.data?.datasets![3].data?.push(Servo);
                    this.chart.update();
                    this.counter = 0;
                }
                this.counter++;
                this.seconds++;
            }
            this.tfirstRow.children[0].textContent = now.toLocaleTimeString("de-DE");
            this.tfirstRow.children[1].textContent = DE_de.format(this.seconds);
            this.tfirstRow.children[2].textContent = DE_de.format(SetpointAirspeed);
            this.tfirstRow.children[3].textContent = DE_de.format(ActualAirspeed);
            this.tfirstRow.children[4].textContent = DE_de.format(Fan);
            this.tfirstRow.children[5].textContent = DE_de.format(Servo);
        };
        xhr.send(ctx.getResult());
    }

    constructor(appManagement:AppManagement, div: HTMLDivElement) {
        super(appManagement, div);
        this.butRecord = <HTMLButtonElement>document.getElementById("airspeedexperiment_butRecord")!;
        this.butStop = <HTMLButtonElement>document.getElementById("airspeedexperiment_butStop")!;
        this.butStop.hidden = true;
        this.butDelete = <HTMLButtonElement>document.getElementById("airspeedexperiment_butDelete")!;
        this.tbody = <HTMLTableSectionElement>document.getElementById("airspeedexperiment_tabBody")!;
        this.tfirstRow = <HTMLTableRowElement>document.getElementById("airspeedexperiment_tabFirstRow")!;
        this.inputSetpointFan = <HTMLInputElement>document.getElementById("airspeedexperiment_inpSetpointFan");
        this.inputServoOL = <HTMLInputElement>document.getElementById("airspeedexperiment_inpFanOL")!;
        this.inputSetpointAirspeed = <HTMLInputElement>document.getElementById("airspeedexperiment_inpSetpointAirspeed");
        this.inputServoCL = <HTMLInputElement>document.getElementById("airspeedexperiment_inpFanCL")!;

        this.inputKP = <HTMLInputElement>document.getElementById("airspeedexperiment_inpKP")!;
        this.inputKI = <HTMLInputElement>document.getElementById("airspeedexperiment_inpKI")!;
        this.inputKD = <HTMLInputElement>document.getElementById("airspeedexperiment_inpKD")!;

        this.onModeChange(0);

        let ctx = <HTMLCanvasElement>document.getElementById('airspeedexperiment_chart')!;
        this.chart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [
                    {
                        label: "Setpoint Airspeed [m/s]",
                        data: [],
                        backgroundColor: "red",
                        borderColor: "red",
                        fill: false,
                    },
                    {
                        label: "Actual Airspeed [m/s]",
                        data: [],
                        backgroundColor: "green",
                        borderColor: "green",
                        fill: false,
                    },
                    {
                        label: "Fan Power [%]",
                        data: [],
                        backgroundColor: "blue",
                        borderColor: "blue",
                        fill: false,
                    },
                    {
                        label: "Servo Position [deg]",
                        data: [],
                        backgroundColor: "grey",
                        borderColor: "grey",
                        fill: false,
                    },
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                hover: {
                    mode: 'nearest',
                    intersect: true
                },
                scales: {
                    y: {
                        beginAtZero: true
                    } 
                }
            }
        });


        document.querySelectorAll('input[name="airspeedexperiment_mode"]').forEach((v, k) => {
            let inp = <HTMLInputElement>v;
            inp.onclick = (e) => {
                let num = parseInt(inp.value);
                if (this.mode != num)
                    this.onModeChange(num);
            };
        });

        let setBubble = (range: HTMLInputElement, bubble: HTMLOutputElement) => {
            let val = range.valueAsNumber;
            let min = range.min ? parseInt(range.min) : 0;
            let max = range.max ? parseInt(range.max) : 100;
            let newVal = ((val - min) * 100) / (max - min);
            bubble.innerHTML = "" + val;

            // Sorta magic numbers based on size of the native UI thumb
            bubble.style.left = `calc(${newVal}% + (${8 - newVal * 0.15}px))`;
        };

        document.querySelectorAll(".range-wrap.airspeedexperiment").forEach(wrap => {
            let range = <HTMLInputElement>wrap.querySelector("input[type='range']")!;
            let bubble = <HTMLOutputElement>wrap.querySelector("output.bubble")!;
            range.oninput = (e) => setBubble(range, bubble);
            setBubble(range, bubble);
        });


        this.butStop.onclick = (e) => {
            this.butStop.hidden = true;
            this.butRecord.hidden = false;
            this.recording = false;
        };

        this.butRecord.onclick = (e) => {
            this.butRecord.hidden = true;
            this.butStop.hidden = false;
            this.recording = true;
        };

        this.butDelete.onclick = (e) => {
            this.resetData();
        };
    }
}
