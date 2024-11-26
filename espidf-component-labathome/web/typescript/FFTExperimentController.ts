import { $ } from "./utils/common";
import { Chart, ChartDataset} from 'chart.js';
import { ScreenController } from "./ScreenController";
import { AppManagement } from "./AppManagement";
import { SerializeContext } from "./flowchart/SerializeContext";
//!!!Applied some adoptions on lower numbers to avoid strange rounding effects
const  FREQUENCIES:number[]=[11,21,31,42,53,64,75,97,118,139,161,183,205,227,258,291,323,355,388,431,474,517,560,614,668,721,786,851,915,991,1066,1152,1238,1335,1443,1550,1669,1798,1938,2089,2239,2401,2584,2778,2982,3198,3435,3682,3951,4231,4533,4856,5200,5566,5965,6385,6837,7321,7838,8398,8990,9625,10304,11025];
const INTERVAL=2000;
const ZEROS = Array.from(Array(64).keys());

export class FFTExperimentController extends ScreenController {
    private butRecord: HTMLButtonElement;
    private butStop: HTMLButtonElement;
    private butSave: HTMLButtonElement;
    private butDelete: HTMLButtonElement;
    private inputFan: HTMLInputElement;
    private tbody: HTMLTableSectionElement;
    private timer: number | undefined;
    private chart: Chart;
    private COLORS = [
        '#4dc9f6',
        '#f67019',
        '#f53794',
        '#537bc4',
        '#acc236',
        '#166a8f',
        '#00a950',
        '#58595b',
        '#8549ba'
      ];

    private recording = false;
   

    public onFirstStart(): void {
        this.timer = window.setInterval(() => { this.sendAndReceive(); }, INTERVAL);
    }
    public onRestart(): void {
        this.timer = window.setInterval(() => { this.sendAndReceive(); }, INTERVAL);
    }
    public onStop(): void {
        window.clearInterval(this.timer);
        this.butStop.hidden = true;
        this.butRecord.hidden = false;
    }
    public onCreate() {
        //this.resetData();
    }

    private resetData() {
        this.chart.data!.labels = [];
        this.chart.data!.datasets!.forEach((dataset) => {
            dataset!.data = [];
        });
        this.chart.update();
        this.tbody.innerText = "";
    }

    private color(index:number) {
        return this.COLORS[index % this.COLORS.length];
    }
      

    private sendAndReceive() {
        let buffer = new ArrayBuffer(256);
        let ctx = new SerializeContext(buffer);
        ctx.writeF32(this.inputFan.valueAsNumber);
        ctx.writeF32(0);
        ctx.writeF32(0);
        ctx.writeF32(0);
        ctx.writeF32(0);
        ctx.writeF32(0);
        ctx.writeF32(0);
        ctx.writeF32(0);

        let xhr = new XMLHttpRequest;
        xhr.onerror = (e) => { console.log("Fehler beim XMLHttpRequest!"); };
        xhr.open("PUT", "/fftexperiment", true);
        xhr.responseType = "arraybuffer";
        
        xhr.onload = (e) => {
            let data:number[] = [];
            let arrayBuffer = xhr.response; // Note: not oReq.responseText
            if (!arrayBuffer || arrayBuffer.byteLength != 256) {
                console.error("!arrayBuffer || arrayBuffer.byteLength != 256 -->providing fake data");
                for(let i =0;i<64;i++){
                    data.push(5 + 5 * Math.random());
                }

            }
            else {
                let ctx = new SerializeContext(arrayBuffer);
                for(let i =0;i<64;i++){
                    data.push(Math.log10(ctx.readF32()));
                }
            }
            if (this.recording) {
                this.chart.data.datasets[0].data=data;
                this.chart.update();
            }
        };
        xhr.send(ctx.getResult());
    }


    constructor(appManagement:AppManagement, div: HTMLDivElement) {
        super(appManagement, div);
        this.butRecord = <HTMLButtonElement>document.getElementById("fftexperiment_butRecord")!;
        this.butStop = <HTMLButtonElement>document.getElementById("fftexperiment_butStop")!;
        this.butSave = <HTMLButtonElement>document.getElementById("fftexperiment_butSave")!;
        this.butDelete = <HTMLButtonElement>document.getElementById("fftexperiment_butDelete")!;
        this.inputFan = <HTMLInputElement>document.getElementById("fftexperiment_inpFan")!;
        this.butStop.hidden = true;
        this.tbody = <HTMLTableSectionElement>document.getElementById("adcexperiment_tabBody")!;
       

        let ctx = <HTMLCanvasElement>document.getElementById('fftexperiment_chart')!;
        this.chart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: FREQUENCIES,
                datasets: []
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
                        beginAtZero: true,
                        max:10
                    } 
                }
            }
        });
        this.chart.update();
        let setBubble = (range: HTMLInputElement, bubble: HTMLOutputElement) => {
            let val = range.valueAsNumber;
            let min = range.min ? parseInt(range.min) : 0;
            let max = range.max ? parseInt(range.max) : 100;
            let newVal = ((val - min) * 100) / (max - min);
            bubble.innerHTML = "" + val;

            // Sorta magic numbers based on size of the native UI thumb
            bubble.style.left = `calc(${newVal}% + (${8 - newVal * 0.15}px))`;
        };

        document.querySelectorAll(".range-wrap.fftexperiment").forEach(wrap => {
            let range = <HTMLInputElement>wrap.querySelector("input[type='range']")!;
            let bubble = <HTMLOutputElement>wrap.querySelector("output.bubble")!;
            range.oninput = (e) => setBubble(range, bubble);
            setBubble(range, bubble);
        });

        this.butStop.onclick = () => {
            this.butStop.hidden = true;
            this.butRecord.hidden = false;
            this.recording = false;
            this.chart.data.datasets.shift();
            this.chart.update();
        };

        this.butRecord.onclick = () => {
            this.butRecord.hidden = true;
            this.butStop.hidden = false;
            this.chart.data.datasets.unshift({
                label: "Magnitudes",
                data: ZEROS,
                borderColor: this.color(0),
                backgroundColor: this.color(0),
            });
            this.recording = true;
            this.chart.update();
        };
        this.butSave.onclick = ()=>{
            let now = new Date(Date.now());
            let newDataset:ChartDataset={
                label: "Magnitudes saved "+now.toLocaleTimeString("de-DE"),
                data: this.chart.data.datasets[0].data,
                borderColor: this.color(this.chart.data.datasets.length),
                backgroundColor: this.color(this.chart.data.datasets.length),
            };
            this.chart.data.datasets.splice(1,0,newDataset);
            this.chart.update();
        }
        this.butDelete.onclick=()=>{
            if(this.chart.data.datasets.length<2) return;
            this.chart.data.datasets.pop();
            this.chart.update();
        }
    }
}
