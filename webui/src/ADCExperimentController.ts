import { $ } from "./Utils";
import Chart from 'chart.js';
import { ScreenController } from "./ScreenController";
import { AppManagement } from "./AppManagement";
import { SerializeContext } from "./flowchart/SerializeContext";

export let DE_de = new Intl.NumberFormat('de-DE');
export const CHART_EACH_INTERVAL = 2;

export class ADCExperimentController extends ScreenController {
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
    private inputKI: HTMLInputElement;
    private inputKD: HTMLInputElement;
    private timer: number | undefined;
    private chart: Chart;
    private chartConfig: Chart.ChartConfiguration;
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
        this.chartConfig.data!.labels = [];
        this.chartConfig.data!.datasets!.forEach((dataset) => {
            dataset!.data = [];
        });
        this.chart.update();
        this.tbody.innerText = "";
        this.seconds = 0;
    }

    private sendAndReceive() {

        let xhr = new XMLHttpRequest;
        xhr.onerror = (e) => { console.log("Fehler beim XMLHttpRequest!"); };
        xhr.open("GET", "/adcexperiment", true);
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
                    if (this.chartConfig.data!.labels!.length > 100) {
                        this.chartConfig.data!.labels?.shift();
                        this.chartConfig.data!.datasets!.forEach((dataset) => {
                            dataset!.data!.shift();
                        });
                    }
                    this.chartConfig.data!.labels!.push(now.toLocaleTimeString("de-DE"));
                    this.chartConfig.data?.datasets![0].data?.push(Values[0]);
                    this.chartConfig.data?.datasets![1].data?.push(Values[1]);
                    this.chartConfig.data?.datasets![2].data?.push(Values[2]);
                    this.chartConfig.data?.datasets![3].data?.push(Values[3]);
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
        xhr.send();
    }

    constructor(appManagement:AppManagement, div: HTMLDivElement) {
        super(appManagement, div);
        this.butRecord = <HTMLButtonElement>document.getElementById("experiment_butRecord")!;
        this.butStop = <HTMLButtonElement>document.getElementById("experiment_butStop")!;
        this.butStop.hidden = true;
        this.butDelete = <HTMLButtonElement>document.getElementById("experiment_butDelete")!;
        this.tbody = <HTMLTableSectionElement>document.getElementById("experiment_tabBody")!;
        this.tfirstRow = <HTMLTableRowElement>document.getElementById("experiment_tabFirstRow")!;
        
        this.chartConfig = {
            type: 'line',
            data: {
                labels: [],
                datasets: [
                    {
                        label: "Input 0 [V]",
                        data: [],
                        backgroundColor: "red",
                        borderColor: "red",
                        fill: false,
                    },
                    {
                        label: "Input 1 [V]",
                        data: [],
                        backgroundColor: "green",
                        borderColor: "green",
                        fill: false,
                    },
                    {
                        label: "Input 2 [V]",
                        data: [],
                        backgroundColor: "blue",
                        borderColor: "blue",
                        fill: false,
                    },
                    {
                        label: "Input 3 [V]",
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
                tooltips: {
                    mode: 'index',
                    intersect: false,
                },
                hover: {
                    mode: 'nearest',
                    intersect: true
                },
                scales: {
                    yAxes: [{
                        ticks: {
                            beginAtZero: true
                        }
                    }]
                }
            }
        };

        let ctx = <HTMLCanvasElement>document.getElementById('experiment_chart')!;
        this.chart = new Chart(ctx, this.chartConfig);

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
