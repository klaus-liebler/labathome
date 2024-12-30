import { $ } from "./utils/common";
import { Chart} from 'chart.js';
import { ScreenController } from "./ScreenController";
import { AppManagement } from "./AppManagement";
import { SerializeContext } from "./flowchart/SerializeContext";

export let DE_de = new Intl.NumberFormat('de-DE');
export const CHART_EACH_INTERVAL = 2;

export class PtnExperimentController extends ScreenController {
    private butRecord: HTMLButtonElement;
    private butStop: HTMLButtonElement;
    private butDelete: HTMLButtonElement;
    private tbody: HTMLTableSectionElement;
    private tfirstRow: HTMLTableRowElement;
    private inputSetpointVoltageY: HTMLInputElement;
    private inputSetpointVoltageX: HTMLInputElement;
    private inputKP: HTMLInputElement;
    private inputTN: HTMLInputElement;
    private inputTV: HTMLInputElement;
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
                document.querySelectorAll('.ptnexperiment_closedloopctrl').forEach((v, k) => {
                    (<HTMLElement>v).style.display = "none";
                });
                document.querySelectorAll('.ptnexperiment_openloopctrl').forEach((v, k) => {
                    (<HTMLElement>v).style.display = "none";
                });
                break;
            case 1:
                document.querySelectorAll('.ptnexperiment_closedloopctrl').forEach((v, k) => {
                    (<HTMLElement>v).style.display = "none";
                });
                document.querySelectorAll('.ptnexperiment_openloopctrl').forEach((v, k) => {
                    (<HTMLElement>v).style.display = "inline-block";
                });
                break;
            case 2:
                document.querySelectorAll('.ptnexperiment_closedloopctrl').forEach((v, k) => {
                    (<HTMLElement>v).style.display = "inline-block";
                });
                document.querySelectorAll('.ptnexperiment_openloopctrl').forEach((v, k) => {
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
        } else if (this.mode == 1) {
            ctx.writeF32(this.inputSetpointVoltageY.valueAsNumber);
        }
        else {
            ctx.writeF32(this.inputSetpointVoltageX.valueAsNumber);
        }
        ctx.writeF32(this.inputKP.valueAsNumber);
        ctx.writeF32(this.inputTN.valueAsNumber);
        ctx.writeF32(this.inputTV.valueAsNumber);

        let xhr = new XMLHttpRequest;
        xhr.onerror = (e) => { console.log("Fehler beim XMLHttpRequest!"); };
        xhr.open("PUT", "/ptnexperiment", true);
        xhr.responseType = "arraybuffer";
        xhr.onload = (e) => {
            let Values: number[]=[0,0,0,0];
            let arrayBuffer = xhr.response; // Note: not oReq.responseText
            if (arrayBuffer || arrayBuffer.byteLength == 4 + 4 + 4 + 4) {
                let ctx = new SerializeContext(arrayBuffer);
                Values[0] = ctx.readF32();
                Values[1] = ctx.readF32();
                Values[2] = ctx.readF32();
                Values[3] = ctx.readF32();
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
                    this.chart.data?.datasets![0].data?.push(Values[0]);
                    this.chart.data?.datasets![1].data?.push(Values[1]);
                    this.chart.data?.datasets![2].data?.push(Values[2]);
                    this.chart.data?.datasets![3].data?.push(Values[3]);
                    this.chart.update();
                    this.counter = 0;
                }
                this.counter++;
                this.seconds++;
            }
            this.tfirstRow.children[0].textContent = now.toLocaleTimeString("de-DE");
            this.tfirstRow.children[1].textContent = DE_de.format(this.seconds);
            this.tfirstRow.children[2].textContent = DE_de.format(Values[0]);
            this.tfirstRow.children[3].textContent = DE_de.format(Values[1]);
            this.tfirstRow.children[4].textContent = DE_de.format(Values[2]);
            this.tfirstRow.children[5].textContent = DE_de.format(Values[3]);
        };
        xhr.send(ctx.getResult());
    }

    constructor(appManagement:AppManagement, div: HTMLDivElement) {
        super(appManagement, div);
        this.butRecord = <HTMLButtonElement>document.getElementById("ptnexperiment_butRecord")!;
        this.butStop = <HTMLButtonElement>document.getElementById("ptnexperiment_butStop")!;
        this.butStop.hidden = true;
        this.butDelete = <HTMLButtonElement>document.getElementById("ptnexperiment_butDelete")!;
        this.tbody = <HTMLTableSectionElement>document.getElementById("ptnexperiment_tabBody")!;
        this.tfirstRow = <HTMLTableRowElement>document.getElementById("ptnexperiment_tabFirstRow")!;


        this.inputSetpointVoltageY = <HTMLInputElement>document.getElementById("ptnexperiment_inpSetpointVoltageY");
        this.inputSetpointVoltageX = <HTMLInputElement>document.getElementById("ptnexperiment_inpSetpointVoltageX");
       

        this.inputKP = <HTMLInputElement>document.getElementById("ptnexperiment_inpKP")!;
        this.inputTN = <HTMLInputElement>document.getElementById("ptnexperiment_inpTN")!;
        this.inputTV = <HTMLInputElement>document.getElementById("ptnexperiment_inpTV")!;

        this.onModeChange(0);

        let ctx = <HTMLCanvasElement>document.getElementById('ptnexperiment_chart')!;
        this.chart = new Chart(ctx,{
            type: 'line',
            data: {
                labels: [],
                datasets: [
                    {
                        label: "Input [V]",
                        data: [],
                        backgroundColor: "red",
                        borderColor: "red",
                        fill: false,
                    },
                    {
                        label: "PT1 [V]",
                        data: [],
                        backgroundColor: "green",
                        borderColor: "green",
                        fill: false,
                    },
                    {
                        label: "PT2 [V]",
                        data: [],
                        backgroundColor: "blue",
                        borderColor: "blue",
                        fill: false,
                    },
                    {
                        label: "PT3 [V]",
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

        document.querySelectorAll('input[name="ptnexperiment_mode"]').forEach((v, k) => {
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

        document.querySelectorAll(".range-wrap.ptnexperiment").forEach(wrap => {
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
