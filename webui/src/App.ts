import {Flowchart, FlowchartData, FlowchartOptions} from "./Flowchart";
import {$} from "./Utils"
import * as chrtst from "chartist";

let DE_de = new Intl.NumberFormat('de-DE');

export abstract class ScreenController {
    private state:ControllerState;
    constructor(protected div: HTMLDivElement) {
        this.hideDIV();
        this.state=ControllerState.CREATED;
    }
    get ElementId() { return this.div.id; }
    get State(){return this.state;}
    set State(value:ControllerState){this.state=value;}
    abstract onCreate():void;
    abstract onFirstStart():void;
    abstract onRestart():void;
    abstract onStop():void;
    public showDIV() {
        this.div.style.display = "block";
    }
    public hideDIV() {
        this.div.style.display = "none";
    }
}

class SerializeContext {
    private bufferDV:DataView;
    constructor(private buffer: ArrayBuffer, private bufferOffset: number=0)
    {
        this.bufferDV=new DataView(buffer);
    }
    public writeS32(theNumber:number):void
    {
        this.bufferDV.setInt32(this.bufferOffset, theNumber, true);
        this.bufferOffset+=4;
    }

    public writeU32(theNumber:number):void
    {
        this.bufferDV.setUint32(this.bufferOffset, theNumber, true);
        this.bufferOffset+=4;
    }

    public writeF32(theNumber:number):void
    {
        this.bufferDV.setFloat32(this.bufferOffset, theNumber, true);
        this.bufferOffset+=4;
    }

    public readF32():number
    {
        let val = this.bufferDV.getFloat32(this.bufferOffset, true);
        this.bufferOffset+=4;
        return val;
    }

    public readU32():number
    {
        let val = this.bufferDV.getUint32(this.bufferOffset, true);
        this.bufferOffset+=4;
        return val;
    }

    public getResult():ArrayBuffer{
        return this.buffer.slice(0, this.bufferOffset)
    }
}

class ExperimentController extends ScreenController {
    private butRecord:HTMLButtonElement;
    private butStop:HTMLButtonElement;
    private butDelete:HTMLButtonElement;
    
    private tbody:HTMLTableSectionElement;
    private tfirstRow:HTMLTableRowElement;
    
    private inputSetpointHeater:HTMLInputElement;
    private inputSetpointTemperature:HTMLInputElement;
    private inputFanCL:HTMLInputElement;
    private inputFanOL:HTMLInputElement;
    private inputKP:HTMLInputElement;
    private inputKI:HTMLInputElement;
    private inputKD:HTMLInputElement;
    private timer:number|undefined;
    private chart:chrtst.IChartistLineChart;
    private chartData:chrtst.IChartistData;
    private counter=10^6;
    private dateValues:string[]=[]
    private actualTemperatureValues:number[] =[];
    private setpointTemperatureValues:number[] =[];
    private heaterValues:number[]=[];
    private fanValues:number[]=[];
    private mode:number=0;
    private seconds=0;

    private recording=false;

    public onFirstStart(): void {
        this.chart.update(this.chartData);
        this.timer = window.setInterval(() => {this.sendAndReceive();}, 1000);
    }
    public onRestart(): void {
        this.chart.update(this.chartData);
        this.timer = window.setInterval(() => {this.sendAndReceive();}, 1000);
    }
    public onStop(): void {
        window.clearInterval(this.timer);
        this.butStop.hidden=true;
        this.butRecord.hidden=false;
        this.counter=10^6;
    }
    public onCreate() {
        this.resetData();
        
    }

    private resetData(){
        this.dateValues=[];
        this.setpointTemperatureValues=[];
        this.actualTemperatureValues=[]
        this.fanValues=[];
        this.heaterValues=[];
        this.tbody.innerText="";
        this.seconds=0;
    }


    private onModeChange(newMode:number){
        switch(newMode){
            case 0:
                document.querySelectorAll('.experiment_closedloopctrl').forEach((v,k)=>{
                    (<HTMLElement>v).style.display = "none";
                });
                document.querySelectorAll('.experiment_openloopctrl').forEach((v,k)=>{
                    (<HTMLElement>v).style.display = "none";
                });
                break;
            case 1:
                document.querySelectorAll('.experiment_closedloopctrl').forEach((v,k)=>{
                    (<HTMLElement>v).style.display = "none";
                });
                document.querySelectorAll('.experiment_openloopctrl').forEach((v,k)=>{
                    (<HTMLElement>v).style.display = "inline-block";
                });
                break;
            case 2:
                document.querySelectorAll('.experiment_closedloopctrl').forEach((v,k)=>{
                    (<HTMLElement>v).style.display = "inline-block";
                });
                document.querySelectorAll('.experiment_openloopctrl').forEach((v,k)=>{
                    (<HTMLElement>v).style.display = "none";
                });
            break;
        }
        this.mode=newMode;
    }

    private sendAndReceive()
    {
        let buffer = new ArrayBuffer(256);
        let ctx=new SerializeContext(buffer);
        //uint32_t modeU32 = bufU32[0];
        //float setpointTempOrHeater = bufF32[1];
        //float setpointFan = bufF32[2];
        //uint32_t KP = bufU32[3];
        //uint32_t KI = bufU32[4];
        //uint32_t KD = bufU32[5];

        ctx.writeU32(this.mode);
        if(this.mode==0){
            ctx.writeF32(0);
            ctx.writeF32(0);
        }else if(this.mode==1)
        {
            ctx.writeF32(this.inputSetpointHeater.valueAsNumber)
            ctx.writeF32(this.inputFanOL.valueAsNumber);
        }
        else{
            ctx.writeF32(this.inputSetpointTemperature.valueAsNumber);
            ctx.writeF32(this.inputFanCL.valueAsNumber);
        }
        ctx.writeF32(this.inputKP.valueAsNumber);
        ctx.writeF32(this.inputKI.valueAsNumber);
        ctx.writeF32(this.inputKD.valueAsNumber);
    
        let xhr = new XMLHttpRequest;
        xhr.onerror=(e)=>{console.log("Fehler beim XMLHttpRequest!")}
        xhr.open("PUT", "/experiment", true);
        xhr.responseType = "arraybuffer";
        xhr.onload=(e)=>{
            let SetpointTemperature:number, Heater:number, Fan:number, ActualTemperature:number;
            let arrayBuffer = xhr.response; // Note: not oReq.responseText
            if (!arrayBuffer || arrayBuffer.byteLength!=4+4+4+4) {
                console.error("! arrayBuffer || arrayBuffer.byteLength!=4+4+4+4");
                SetpointTemperature=0;
                Heater=0;
                Fan=0;
                ActualTemperature=0;
            }
            else{
                let ctx=new SerializeContext(arrayBuffer);
                //float retbuf[4];
                //retbuf[0]=returnData.SetpointTemperature;
                //retbuf[1]=returnData.Heater;
                //retbuf[2]=returnData.Fan;
                //retbuf[3]=returnData.ActualTemperature;
                SetpointTemperature = ctx.readF32();
                Heater = ctx.readF32();
                Fan = ctx.readF32();
                ActualTemperature = ctx.readF32();
            }
            let now = new Date(Date.now());
            
            if(this.recording)
            {
                let tr = $.HtmlAsFirstChild(this.tbody, "tr", []);
                $.Html(tr, "td", [], [], this.tfirstRow.children[0].textContent!);
                $.Html(tr, "td", [], [], this.tfirstRow.children[1].textContent!);
                $.Html(tr, "td", [], [], this.tfirstRow.children[2].textContent!);
                $.Html(tr, "td", [], [], this.tfirstRow.children[3].textContent!);
                $.Html(tr, "td", [], [], this.tfirstRow.children[4].textContent!);
                $.Html(tr, "td", [], [], this.tfirstRow.children[5].textContent!);
                if(this.counter>=5)
                {
                    if(this.dateValues.length>100)
                    {
                        this.dateValues=this.dateValues.slice(1);
                        this.setpointTemperatureValues.slice(1);
                        this.heaterValues.slice(1);
                        this.fanValues.slice(1);
                        this.actualTemperatureValues.slice(1);
                    }
                    this.dateValues.push(now.toLocaleTimeString("de-DE"));
                    this.setpointTemperatureValues.push(SetpointTemperature)
                    this.heaterValues.push(Heater);
                    this.fanValues.push(Fan);
                    this.actualTemperatureValues.push(ActualTemperature);
                    this.chartData = {
                        labels:this.dateValues,
                        series: [this.setpointTemperatureValues, this.actualTemperatureValues, this.heaterValues, this.fanValues,],
                    };
                    this.chart.update(this.chartData);
                    this.counter=0;
                }
                this.counter++;
                this.seconds++;
            }

            
            this.tfirstRow.children[0].textContent=now.toLocaleTimeString("de-DE");
            this.tfirstRow.children[1].textContent=DE_de.format(this.seconds);
            this.tfirstRow.children[2].textContent=DE_de.format(SetpointTemperature);
            this.tfirstRow.children[3].textContent=DE_de.format(Heater);
            this.tfirstRow.children[4].textContent=DE_de.format(Fan);
            this.tfirstRow.children[5].textContent=DE_de.format(ActualTemperature);
            
            
        }
        xhr.send(ctx.getResult());
    }

    constructor(public div: HTMLDivElement) {
        super(div);
        this.butRecord=<HTMLButtonElement>document.getElementById("experiment_butRecord")!;
        this.butStop=<HTMLButtonElement>document.getElementById("experiment_butStop")!;
        this.butStop.hidden=true;
        this.butDelete=<HTMLButtonElement>document.getElementById("experiment_butDelete")!;
        this.tbody=<HTMLTableSectionElement>document.getElementById("experiment_tabBody")!;
        this.tfirstRow=<HTMLTableRowElement>document.getElementById("experiment_tabFirstRow")!;
        this.inputSetpointHeater=<HTMLInputElement>document.getElementById("experiment_inpSetpointHeater");
        this.inputFanOL=<HTMLInputElement>document.getElementById("experiment_inpFanOL")!;
        this.inputSetpointTemperature=<HTMLInputElement>document.getElementById("experiment_inpSetpointTemperature");
        this.inputFanCL=<HTMLInputElement>document.getElementById("experiment_inpFanCL")!;

        this.inputKP=<HTMLInputElement>document.getElementById("experiment_inpKP")!;
        this.inputKI=<HTMLInputElement>document.getElementById("experiment_inpKI")!;
        this.inputKD=<HTMLInputElement>document.getElementById("experiment_inpKD")!;

        this.onModeChange(0);


        this.chartData = {
            labels:this.dateValues,
            series: [this.setpointTemperatureValues, this.actualTemperatureValues, this.heaterValues, this.fanValues,],
        };
        let options:chrtst.ILineChartOptions={
            axisX:{
                labelInterpolationFnc:(value:any, index:number)=>{return index % 5 === 0 ? value : null;}
            }
        };
        this.chart= new chrtst.Line('#experiment_chart', this.chartData, options);

        document.querySelectorAll('input[name="experiment_mode"]').forEach((v,k)=>{
            let inp =<HTMLInputElement>v;
            inp.onclick=(e)=>{
                let num = parseInt(inp.value)
                if(this.mode!=num) this.onModeChange(num);
            }
        });

        let setBubble = (range:HTMLInputElement, bubble:HTMLOutputElement)=> {
            let val = range.valueAsNumber;
            let min = range.min ? parseInt(range.min) : 0;
            let max = range.max ? parseInt(range.max) : 100;
            let newVal = ((val - min) * 100) / (max - min);
            bubble.innerHTML = ""+val;
          
            // Sorta magic numbers based on size of the native UI thumb
            bubble.style.left = `calc(${newVal}% + (${8 - newVal * 0.15}px))`;
        }

        document.querySelectorAll(".range-wrap").forEach(wrap => {
            let range = <HTMLInputElement>wrap.querySelector("input[type='range']")!;
            let bubble = <HTMLOutputElement>wrap.querySelector("output.bubble")!;
            range.oninput=(e)=>setBubble(range, bubble);
            setBubble(range, bubble);
        });
        

        this.butStop.onclick=(e)=>{
            this.butStop.hidden=true;
            this.butRecord.hidden=false;
            this.recording=false;    
        }

        this.butRecord.onclick=(e)=>
        {
            this.butRecord.hidden=true;
            this.butStop.hidden=false;
            this.recording=true;
        }

        this.butDelete.onclick=(e)=>
        {
            this.resetData();
        }
    }
}

export class DevelopCFCController extends ScreenController {
    private fc:Flowchart;
    onFirstStart(): void {
       this.fc.onFirstStart();
    }
    public onRestart(): void {}
    public onStop(): void {}
    public onCreate() {}
    constructor(public div: HTMLDivElement) {
        super(div);
        let data: FlowchartData = {
            operators: [
                {
                    index:0,
                    caption: "RedButton_1",
                    typeName: "RedButton",
                    posX: 10,
                    posY: 10,
                    configurationData:null,
                },
                {
                    index:1,
                    caption: "GreenButton_1",
                    typeName: "GreenButton",
                    posX: 10,
                    posY: 150,
                    configurationData:null,
                },
                {
                    index:2,
                    caption: "AND_1",
                    typeName: "AND",
                    posX: 250,
                    posY: 10,
                    configurationData:null,
                },
                {
                    index:3,
                    caption: "RedLed_1",
                    typeName: "RedLed",
                    posX: 500,
                    posY: 10,
                    configurationData:null,
                },
               
            ],
            links: [
                {
                    color: "black",
                    fromOperatorIndex: 0,
                    fromOutput: 0,
                    toOperatorIndex: 2,
                    toInput: 0
                },
                {
                    color: "black",
                    fromOperatorIndex: 1,
                    fromOutput: 0,
                    toOperatorIndex: 2,
                    toInput: 1
                },
                {
                    color: "black",
                    fromOperatorIndex: 2,
                    fromOutput: 0,
                    toOperatorIndex: 3,
                    toInput: 0
                },
            ]
        };
        let options = new FlowchartOptions();
        options.data = data;
        this.fc = new Flowchart(this.div, options);
    }
    

}

class DashboardController extends ScreenController {
    public onFirstStart(): void {}
    public onRestart(): void {}
    public onStop(): void {}
    public onCreate() {}
    constructor(public div: HTMLDivElement) {
        super(div);
    }

}

class ReportsController extends ScreenController {
    public onFirstStart(): void {}
    public onRestart(): void {}
    public onStop(): void {}
    constructor(public div: HTMLDivElement) {
        super(div);
    }
    public onCreate() {
        return;

    }
}

enum ControllerState{
    CREATED,
    STARTED,
    STOPPED,
}

class AppController {

    private stateDiv: HTMLDivElement;
    private activeControllerIndex:number;
    private screenControllers:ScreenController[];

    

    constructor() {
        this.stateDiv = <HTMLDivElement>document.getElementById("spnConnectionState")!;
        this.screenControllers=[];
        this.activeControllerIndex=0;
    }

    private SetApplicationState(state: string) {
        this.stateDiv.innerHTML = state;

    }

    private setActiveScreen(newIndex:number)
    {
        this.screenControllers.forEach((controller,i)=>{
            if(i==newIndex)
            {
                controller.showDIV();
                if(controller.State==ControllerState.CREATED){
                    controller.onFirstStart();
                    controller.State=ControllerState.STARTED;
                }
                else{
                    controller.onRestart();
                    controller.State=ControllerState.STARTED;
                }
            }else{
                controller.hideDIV();
                if(controller.State==ControllerState.STARTED)
                {
                    controller.onStop();
                    controller.State=ControllerState.STOPPED;
                }
            }
        });
        this.activeControllerIndex=newIndex;
    }

    public startup() {
        this.screenControllers.push(new DashboardController(<HTMLDivElement>document.getElementById("screen_dashboard")));
        this.screenControllers.push(new DevelopCFCController(<HTMLDivElement>document.getElementById("screen_develop")));
        this.screenControllers.push(new ReportsController(<HTMLDivElement>document.getElementById("screen_reports")));
        this.screenControllers.push(new ExperimentController(<HTMLDivElement>document.getElementById("screen_experiment")));
        this.screenControllers.forEach((sc)=>sc.onCreate());
        
        this.setActiveScreen(0);
        let id2index = new Map<string, number>();
        this.screenControllers.forEach((value, index)=>{id2index.set("show_"+value.ElementId, index)})
        document.querySelectorAll<HTMLAnchorElement>("nav a").forEach((a: HTMLAnchorElement) => {
            let id = a.id;
            let index=id2index.get(a.id)||0;
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


