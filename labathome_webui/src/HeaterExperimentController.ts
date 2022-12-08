import { $ } from "./utils";
import { Chart} from 'chart.js';
import { ScreenController } from "./ScreenController";
import { AppManagement } from "./AppManagement";
import { SerializeContext } from "./flowchart/SerializeContext";

export let DE_de = new Intl.NumberFormat('de-DE');
export const CHART_EACH_INTERVAL = 2;

export class HeaterExperimentController extends ScreenController {
    private butRecord: HTMLButtonElement;
    private butStop: HTMLButtonElement;
    private butDelete: HTMLButtonElement;
    private tbody: HTMLTableSectionElement;
    private tfirstRow: HTMLTableRowElement;
    private inputSetpointHeater: HTMLInputElement;
    private inputSetpointTemperature: HTMLInputElement;
    private inputFanCL: HTMLInputElement;
    private inputFanOL: HTMLInputElement;
    private inputKP: HTMLInputElement;
    private inputTN: HTMLInputElement;
    private inputTV: HTMLInputElement;
    private inputReset:HTMLInputElement;
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
                document.querySelectorAll('.heaterexperiment_closedloopctrl').forEach((v, k) => {
                    (<HTMLElement>v).style.display = "none";
                });
                document.querySelectorAll('.heaterexperiment_openloopctrl').forEach((v, k) => {
                    (<HTMLElement>v).style.display = "none";
                });
                break;
            case 1:
                document.querySelectorAll('.heaterexperiment_closedloopctrl').forEach((v, k) => {
                    (<HTMLElement>v).style.display = "none";
                });
                document.querySelectorAll('.heaterexperiment_openloopctrl').forEach((v, k) => {
                    (<HTMLElement>v).style.display = "inline-block";
                });
                break;
            case 2:
                document.querySelectorAll('.heaterexperiment_closedloopctrl').forEach((v, k) => {
                    (<HTMLElement>v).style.display = "inline-block";
                });
                document.querySelectorAll('.heaterexperiment_openloopctrl').forEach((v, k) => {
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
            ctx.writeF32(this.inputSetpointHeater.valueAsNumber);
            ctx.writeF32(this.inputFanOL.valueAsNumber);
        }
        else {
            ctx.writeF32(this.inputSetpointTemperature.valueAsNumber);
            ctx.writeF32(this.inputFanCL.valueAsNumber);
        }
        ctx.writeF32(this.inputKP.valueAsNumber);
        ctx.writeF32(this.inputTN.valueAsNumber);
        ctx.writeF32(this.inputTV.valueAsNumber);
        ctx.writeU32(this.inputReset.checked?1:0);
        let xhr = new XMLHttpRequest;
        xhr.onerror = (e) => { console.log("Fehler beim XMLHttpRequest!"); };
        xhr.open("PUT", "/heaterexperiment", true);
        xhr.responseType = "arraybuffer";
        xhr.onload = (e) => {
            let SetpointTemperature: number, Heater: number, Fan: number, ActualTemperature: number;
            let arrayBuffer = xhr.response; // Note: not oReq.responseText
            if (!arrayBuffer || arrayBuffer.byteLength != 4 + 4 + 4 + 4) {
                console.error("! arrayBuffer || arrayBuffer.byteLength!=4+4+4+4");
                SetpointTemperature = 0;
                Heater = 0;
                Fan = 0;
                ActualTemperature = 20 + (-5 + 10 * Math.random());
            }
            else {
                let ctx = new SerializeContext(arrayBuffer);
                SetpointTemperature = ctx.readF32();
                Heater = ctx.readF32();
                Fan = ctx.readF32();
                ActualTemperature = ctx.readF32();
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
                    this.chart.data?.datasets![0].data?.push(SetpointTemperature);
                    this.chart.data?.datasets![1].data?.push(ActualTemperature);
                    this.chart.data?.datasets![2].data?.push(Heater);
                    this.chart.data?.datasets![3].data?.push(Fan);
                    //this.setpointTemperatureValues.push(SetpointTemperature)
                    //this.heaterValues.push(Heater);
                    //this.fanValues.push(Fan);
                    //this.actualTemperatureValues.push(ActualTemperature);
                    this.chart.update();
                    //FIXME this.chartData = {labels:this.dateValues, series: [this.setpointTemperatureValues, this.actualTemperatureValues, this.heaterValues, this.fanValues,],};
                    //FIXME this.chart.update(this.chartData);
                    this.counter = 0;
                }
                this.counter++;
                this.seconds++;
            }

            this.tfirstRow.children[0].textContent = now.toLocaleTimeString("de-DE");
            this.tfirstRow.children[1].textContent = DE_de.format(this.seconds);
            this.tfirstRow.children[2].textContent = DE_de.format(SetpointTemperature);
            this.tfirstRow.children[3].textContent = DE_de.format(ActualTemperature);
            this.tfirstRow.children[4].textContent = DE_de.format(Heater);
            this.tfirstRow.children[5].textContent = DE_de.format(Fan);
        };
        xhr.send(ctx.getResult());
    }

    constructor(appManagement:AppManagement, div: HTMLDivElement) {
        super(appManagement, div);
        this.butRecord = <HTMLButtonElement>document.getElementById("heaterexperiment_butRecord")!;
        this.butStop = <HTMLButtonElement>document.getElementById("heaterexperiment_butStop")!;
        this.butStop.hidden = true;
        this.butDelete = <HTMLButtonElement>document.getElementById("heaterexperiment_butDelete")!;
        this.tbody = <HTMLTableSectionElement>document.getElementById("heaterexperiment_tabBody")!;
        this.tfirstRow = <HTMLTableRowElement>document.getElementById("heaterexperiment_tabFirstRow")!;
        this.inputSetpointHeater = <HTMLInputElement>document.getElementById("heaterexperiment_inpSetpointHeater");
        this.inputFanOL = <HTMLInputElement>document.getElementById("heaterexperiment_inpFanOL")!;
        this.inputSetpointTemperature = <HTMLInputElement>document.getElementById("heaterexperiment_inpSetpointTemperature");
        this.inputFanCL = <HTMLInputElement>document.getElementById("heaterexperiment_inpFanCL")!;

        this.inputKP = <HTMLInputElement>document.getElementById("heaterexperiment_inpKP")!;
        this.inputTN = <HTMLInputElement>document.getElementById("heaterexperiment_inpTN")!;
        this.inputTV = <HTMLInputElement>document.getElementById("heaterexperiment_inpTV")!;
        this.inputReset=<HTMLInputElement>document.getElementById("heaterexperiment_inpReset")!;

        this.onModeChange(0);

        let ctx = <HTMLCanvasElement>document.getElementById('heaterexperiment_chart')!;
        this.chart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [
                    {
                        label: "Setpoint Temperature [°C]",
                        data: [],
                        backgroundColor: "red",
                        borderColor: "red",
                        fill: false,
                    },
                    {
                        label: "Actual Temperature [°C]",
                        data: [],
                        backgroundColor: "green",
                        borderColor: "green",
                        fill: false,
                    },
                    {
                        label: "Heater Power [%]",
                        data: [],
                        backgroundColor: "blue",
                        borderColor: "blue",
                        fill: false,
                    },
                    {
                        label: "Fan Power [%]",
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


        document.querySelectorAll('input[name="heaterexperiment_mode"]').forEach((v, k) => {
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

        document.querySelectorAll(".range-wrap.heaterexperiment").forEach(wrap => {
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
