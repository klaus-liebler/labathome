
import { CategoryScale, Chart, LinearScale, LineController, LineElement, PointElement } from 'chart.js';
import { ScreenController } from './screen_controller';
import { Html, HtmlAsFirstChild } from '../utils/common';
import { IAppManagement } from '../utils/interfaces';
import { html } from 'lit-html';
import { ByteBuffer } from 'flatbuffers';
import { createRef, ref, Ref } from 'lit-html/directives/ref.js';
import * as flatbuffers from 'flatbuffers';
import { Mode, Namespace, RequestHeater, ResponseHeater } from "../../generated/flatbuffers/heaterexperiment";


export let DE_de = new Intl.NumberFormat('de-DE');
export const CHART_EACH_INTERVAL = 2;

export class HeaterExperimentController extends ScreenController {

    private butRecord: Ref<HTMLButtonElement> = createRef();
    private butStop: Ref<HTMLButtonElement> = createRef();
    private butDelete: Ref<HTMLButtonElement> = createRef();
    private tbody: Ref<HTMLTableSectionElement> = createRef();
    private tfirstRow: Ref<HTMLTableRowElement> = createRef();
    private inputSetpointHeater: Ref<HTMLInputElement> = createRef();
    private inputSetpointTemperature: Ref<HTMLInputElement> = createRef();
    private inputFanCL: Ref<HTMLInputElement> = createRef();
    private inputFanOL: Ref<HTMLInputElement> = createRef();
    private inputKP: Ref<HTMLInputElement> = createRef();
    private inputTN: Ref<HTMLInputElement> = createRef();
    private inputTV: Ref<HTMLInputElement> = createRef();
    private inputReset: Ref<HTMLInputElement> = createRef();
    private inputWP: Ref<HTMLInputElement> = createRef();

    private frmOpenloop: Ref<HTMLFormElement> = createRef();
    private frmClosedloop: Ref<HTMLFormElement> = createRef();

    private timer: number | undefined;
    private chart: Chart;
    private chartCanvas: Ref<HTMLCanvasElement> = createRef();

    private counter = 10 ^ 6;
    private mode: Mode = Mode.FunctionBlock;
    private seconds = 0;

    private recording = false;

    public Template = () => html`
     <form>
            <label>Main Controls</label>
            <div class="mode_toggler">
              <label>
                <input type="radio" name="heaterexperiment_mode" @click=${() => {this.onModeChange(Mode.FunctionBlock)}} checked />
                <div class="functionblock box">
                  <span>Functionblock</span>
                </div>
              </label>

              <label>
                <input type="radio" name="heaterexperiment_mode" @click=${() => {this.onModeChange(Mode.OpenLoop)}}  />
                <div class="openloop box">
                  <span>Open Loop</span>
                </div>
              </label>

              <label>
                <input type="radio" name="heaterexperiment_mode" @click=${() => {this.onModeChange(Mode.ClosedLoop)}}  />
                <div class="closedloop box">
                  <span>Closed Loop</span>
                </div>
              </label>

              <button ${ref(this.butRecord)} @click=${() => {
            this.butRecord.value!.hidden = true;
            this.butStop.value!.hidden = false;
            this.recording = true;
        }} class="large" type="button">🔴</button>
              <button ${ref(this.butStop)} @click=${() => {
            this.butStop.value!.hidden = true;
            this.butRecord.value!.hidden = false;
            this.recording = false;
        }} class="large" type="button">⬛</button>
              <button ${ref(this.butDelete)}  @click=${() => {
            this.resetData();
        }} class="large" type="button">🗑️</button>
            </div>
    </form>
    <form ${ref(this.frmOpenloop)}>
            <label class="heaterexperiment_openloopctrl">Heater Power [%]</label>
            <div class="range-wrap heaterexperiment heaterexperiment_openloopctrl">
              <input type="range" ${ref(this.inputSetpointHeater)} @input=${(ev:InputEvent)=>{this.setMyBubble(ev.currentTarget as HTMLInputElement)}}  min="0" max="100" value="0" />
              <output class="bubble"></output>
            </div>
            <label class="heaterexperiment_openloopctrl">Fan Power [%]</label>
            <div class="range-wrap heaterexperiment heaterexperiment_openloopctrl">
              <input type="range" ${ref(this.inputFanOL)} @input=${(ev:InputEvent)=>{this.setMyBubble(ev.currentTarget as HTMLInputElement)}} min="0" max="100" value="0" />
              <output class="bubble"></output>
            </div>
    </form>
    <form ${ref(this.frmClosedloop)}>
            <label class="heaterexperiment_closedloopctrl">Setp. Temp [°C]</label>
            <div class="range-wrap heaterexperiment heaterexperiment_closedloopctrl">
              <input type="range" ${ref(this.inputSetpointTemperature)} @input=${(ev:InputEvent)=>{this.setMyBubble(ev.currentTarget as HTMLInputElement)}} min="0" max="80" value="0" />
              <output class="bubble"></output>
            </div>

            <label class="heaterexperiment_closedloopctrl">Fan Power [%]</label>
            <div class="range-wrap heaterexperiment heaterexperiment_closedloopctrl">
              <input type="range" ${ref(this.inputFanCL)} min="0" max="100" value="0" />
              <output class="bubble"></output>
            </div>

            <label class="heaterexperiment_closedloopctrl">K<sub>P</sub></label>
            <input class="heaterexperiment_closedloopctrl" type="number" ${ref(this.inputKP)} min="0" max="20"
              value="0" step="0.1" />

            <label class="heaterexperiment_closedloopctrl">T<sub>N</sub></label>
            <input class="heaterexperiment_closedloopctrl" type="number" ${ref(this.inputTN)} min="0" max="1"
              value="0" step="0.01" />

            <label class="heaterexperiment_closedloopctrl">T<sub>V</sub></label>
            <input class="heaterexperiment_closedloopctrl" type="number" ${ref(this.inputTV)} min="0" max="100"
              value="0" step="0.25" />
            <label class="heaterexperiment_closedloopctrl">Reset</label>
            <input class="heaterexperiment_closedloopctrl" type="checkbox" ${ref(this.inputReset)} checked />
            <label class="heaterexperiment_closedloopctrl">Working Point Offset</label>
            <input class="heaterexperiment_closedloopctrl" type="number" ${ref(this.inputWP)} min="0" max="100"
              value="0" step="0.25" />
          </form>
          <!-- <div id="experiment_chart" style="height: 300px;" class="ct-chart ct-perfect-fourth"></div>-->
          <div style="height: 300px; width: 100%;"><canvas ${ref(this.chartCanvas)}
              style="height: 300px; width: 100%;"></canvas></div>
          <table>
            <thead>
              <tr>
                <th>Time [HH:MM:SS]</th>
                <th>Time [seconds]</th>
                <th>Setpoint Temperature [°C]</th>
                <th>Actual Temperature [°C]</th>
                <th>Heater [%]</th>
                <th>Fan [%]</th>

              </tr>
              <tr ${ref(this.tfirstRow)}>
                <td></td>
                <td></td>
                <td></td>
                <td></td>
                <td></td>
                <td></td>
              </tr>
            </thead>
            <tbody ${ref(this.tbody)}>
            </tbody>
          </table>
    `

private setMyBubble(range: HTMLInputElement){
    let bubble = <HTMLOutputElement>range.nextElementSibling;
    let val = range.valueAsNumber;
    let min = range.min ? parseInt(range.min) : 0;
    let max = range.max ? parseInt(range.max) : 100;
    let newVal = ((val - min) * 100) / (max - min);
    bubble.innerHTML = "" + val;

    // Sorta magic numbers based on size of the native UI thumb
    bubble.style.left = `calc(${newVal}% + (${8 - newVal * 0.15}px))`;
};
    

public OnFirstStart(): void {
        
        this.timer = window.setInterval(() => { this.sendAndReceive(); }, 1000);
        this.onModeChange(0);
        this.chart = new Chart(this.chartCanvas.value!, {
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
        this.resetData();

    }

    public OnRestart(): void {
        this.OnFirstStart();
    }

    public OnPause(): void {
        window.clearInterval(this.timer);
        this.butStop.value!.hidden = true;
        this.butRecord.value!.hidden = false;
        this.counter = 10 ^ 6;
    }

    public OnCreate() {
        this.appManagement.RegisterWebsocketMessageNamespace(this, Namespace.Value);
    }


    public OnMessage(namespace: number, bb: ByteBuffer): void {
        if(namespace!=Namespace.Value) return;
        let r = ResponseHeater.getRootAsResponseHeater(bb);

              
        var now = new Date(Date.now());

        if (this.recording) {
            let tr = HtmlAsFirstChild(this.tbody.value!, "tr", []);
            for (let i = 0; i < 6; i++) {
                Html(tr, "td", [], [], this.tfirstRow.value!.children[i].textContent!);
            }
            if (this.counter >= CHART_EACH_INTERVAL) {
                if (this.chart.data!.labels!.length > 100) {
                    this.chart.data!.labels?.shift();
                    this.chart.data!.datasets!.forEach((dataset) => {
                        dataset!.data!.shift();
                    });
                }
                this.chart.data!.labels!.push(now.toLocaleTimeString("de-DE"));
                this.chart.data?.datasets![0].data?.push(r.setpointTemperatureDegrees());
                this.chart.data?.datasets![1].data?.push(r.actualTemperatureDegrees());
                this.chart.data?.datasets![2].data?.push(r.heaterPowerPercent());
                this.chart.data?.datasets![3].data?.push(r.fanSpeedPercent());
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

        this.tfirstRow.value!.children[0].textContent = now.toLocaleTimeString("de-DE");
        this.tfirstRow.value!.children[1].textContent = DE_de.format(this.seconds);
        this.tfirstRow.value!.children[2].textContent = DE_de.format(r.setpointTemperatureDegrees());
        this.tfirstRow.value!.children[3].textContent = DE_de.format(r.actualTemperatureDegrees());
        this.tfirstRow.value!.children[4].textContent = DE_de.format(r.heaterPowerPercent());
        this.tfirstRow.value!.children[5].textContent = DE_de.format(r.fanSpeedPercent());
    }

    
    private resetData() {
        this.chart.data!.labels = [];
        this.chart.data!.datasets!.forEach((dataset) => {
            dataset!.data = [];
        });
        this.chart.update();
        this.tbody.value!.innerText = "";
        this.seconds = 0;
    }
    
    private onModeChange(newMode: Mode) {
        if(newMode==this.mode) return;
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
        let b = new flatbuffers.Builder(1024);
        b.finish(RequestHeater.createRequestHeater(b, this.mode,
            this.inputSetpointHeater.value!.valueAsNumber,
            this.inputSetpointTemperature.value!.valueAsNumber,
            this.mode==Mode.OpenLoop?this.inputFanOL.value!.valueAsNumber:this.inputFanCL.value!.valueAsNumber,
            this.inputKP.value!.valueAsNumber,
            this.inputTN.value!.valueAsNumber,
            this.inputTV.value!.valueAsNumber,
            this.inputWP.value!.valueAsNumber,
            this.inputReset.value!.checked ? true : false
         ))
        this.appManagement.WrapAndSend(Namespace.Value, b, 0);
    }
                

    constructor(appManagement: IAppManagement) {
        super(appManagement);
        Chart.register(LinearScale, LineController, CategoryScale, PointElement, LineElement)
    }
}
