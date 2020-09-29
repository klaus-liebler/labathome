import {Flowchart, FlowchartData, FlowchartOptions} from "./Flowchart";
import {$} from "./Utils"
import * as chrtst from "chartist";

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
    private butSet:HTMLButtonElement;
    private butRecord:HTMLButtonElement;
    private butStop:HTMLButtonElement;
    
    private tbody:HTMLTableSectionElement;
    private inputHeater:HTMLInputElement;
    private inputFan:HTMLInputElement;
    private timer:number|undefined;
    private chart:chrtst.IChartistLineChart|undefined;
    private chartData:chrtst.IChartistData;
    private chartOptions:chrtst.ILineChartOptions;

    public onFirstStart(): void {
        
    }
    public onRestart(): void {
       
    }
    public onStop(): void {
        window.clearInterval(this.timer);
        this.butStop.hidden=true;
        this.butRecord.hidden=false;
    }
    public onCreate() {
        

        let currVal = 20;

        for (let i = 0; i < 100; i++) {
            (<string[]>this.chartData.labels).push(""+ (100 - i));
            (<number[][]>this.chartData.series)[0][i] = currVal;
        }
        // Create a new line chart object where as first parameter we pass in a selector
        // that is resolving to our chart container element. The Second parameter
        // is the actual data object.
        this.chart= new chrtst.Line('#experiment_chart', this.chartData, this.chartOptions);
    }
    constructor(public div: HTMLDivElement) {
        super(div);
        this.butSet=<HTMLButtonElement>document.getElementById("experiment_butSet")!;
        this.butRecord=<HTMLButtonElement>document.getElementById("experiment_butRecord")!;
        this.butStop=<HTMLButtonElement>document.getElementById("experiment_butStop")!;
        this.butStop.hidden=true;
        this.tbody=<HTMLTableSectionElement>document.getElementById("experiment_tabBody")!;
        this.inputHeater=<HTMLInputElement>document.getElementById("experiment_inpHeater");
        this.inputFan=<HTMLInputElement>document.getElementById("experiment_inpFan")!;
        this.chartData = {
            // A labels array that can contain any sort of values
            labels:[],
            // Our series array that contains series objects or in this case series data arrays
            series: [[]],
        };
        this.chartOptions = {
            width: 800,
            height: 300
        };
        
        this.butSet.onclick=(e)=>{
            let buffer = new ArrayBuffer(256);
            let ctx=new SerializeContext(buffer);
            ctx.writeF32(this.inputHeater.valueAsNumber);
            ctx.writeF32(this.inputFan.valueAsNumber);
            var xhr = new XMLHttpRequest;
            xhr.open("PUT", "/experiment", true);
            xhr.onloadend=(e)=>{console.log("Erfolgreich hochgeladen")}
            xhr.onerror=(e)=>{window.alert("Fehler: "+e);}
            xhr.send(ctx.getResult());
        }

        this.butStop.onclick=(e)=>{
            this.butStop.hidden=true;
            this.butRecord.hidden=false;
            window.clearInterval(this.timer);
        }

        this.butRecord.onclick=(e)=>
        {
            this.butRecord.hidden=true;
            this.butStop.hidden=false;
            this.timer = window.setInterval(() => {
                var xhr = new XMLHttpRequest;
                xhr.open("GET", "/experiment", true);
                xhr.responseType = "arraybuffer";
                xhr.onload=(e)=>{
                    let time:number, heater:number, fan:number, temp:number;
                    let arrayBuffer = xhr.response; // Note: not oReq.responseText
                    if (!arrayBuffer || arrayBuffer.byteLength!=4+4+4+4) {
                        console.error("! arrayBuffer || arrayBuffer.byteLength!=4+4+4+4");
                        time=Date.now();
                        heater=0;
                        fan=0;
                        temp=99;
                    }
                    else{
                        let ctx=new SerializeContext(arrayBuffer);
                        time = ctx.readU32();
                        heater = ctx.readF32();
                        fan = ctx.readF32();
                        temp = ctx.readF32();
                    }
                    
    
                    let tr = $.HtmlAsFirstChild(this.tbody, "tr", []);
                    $.Html(tr, "td", [], [], `${time}`);
                    $.Html(tr, "td", [], [], `${heater}`);
                    $.Html(tr, "td", [], [], `${fan}`);
                    $.Html(tr, "td", [], [], `${temp}`);
                    let foo = (<number[][]>this.chartData.series)[0].slice(1);
                    foo.push(temp);
                    (<number[][]>this.chartData.series)[0] = foo;
                    this.chart!.update(this.chartData, this.chartOptions, false);
                }
                xhr.send(null);
    
            }, 1000);

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


        this.SetApplicationState("WebSocket is not connected");
        /*
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


