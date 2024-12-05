import { html } from "lit-html";
import { Flowchart, FlowchartCallback, FlowchartOptions } from "../flowchart/Flowchart";
import { FlowchartData } from "../flowchart/FlowchartData";
import { IAppManagement } from "../utils/interfaces";
import { ScreenController } from "./screen_controller";
import { createRef, ref, Ref } from "lit-html/directives/ref.js";
import { ByteBuffer } from "flatbuffers";



export class DevelopCFCController extends ScreenController {
    
    OnMessage(namespace: number, bb: ByteBuffer): void {
        this.fc.OnMessage(namespace,bb);
        
    }
    private mainDiv:Ref<HTMLInputElement> = createRef();
    private fc: Flowchart;
    private timer: number | undefined;

    public Template = () => html`<div ${ref(this.mainDiv)} class="develop-ui"></div>`
    

    OnFirstStart(): void {
        this.timer = window.setInterval(() => { this.fc.TriggerDebug();}, 1000);
        this.fc.RenderUi(this.mainDiv.value!);
    }
    OnRestart(): void {
        this.OnFirstStart()
    }
    OnPause(): void {
        window.clearInterval(this.timer);
    }
    public OnCreate() { }
    
    constructor(appManagement:IAppManagement) {
        super(appManagement);
        let data: FlowchartData = {operators:[],links:[]};
        let options = new FlowchartOptions();
        let callbacks = new FlowchartCallback();
        this.fc = new Flowchart(this.appManagement, data, callbacks, options);
    }
}
