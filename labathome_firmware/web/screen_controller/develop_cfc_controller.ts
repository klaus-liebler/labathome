import { html } from "lit-html";
import { Flowchart, FlowchartCallback, FlowchartOptions } from "../flowchart/Flowchart";
import { FlowchartData } from "../flowchart/FlowchartData";
import { IAppManagement } from "@klaus-liebler/web-components/typescript/utils/interfaces";
import { ScreenController } from "@klaus-liebler/web-components/typescript/screen_controller/screen_controller";
import { createRef, ref, Ref } from "lit-html/directives/ref.js";
import { functionblock } from "@generated/wsprotocol_ts/ws-protocol";

export class DevelopCFCController extends ScreenController {

    OnMessage(namespaceId: number, messageTypeId: number, view: DataView): void {
        this.fc.OnMessage(namespaceId, messageTypeId, view);
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

    constructor(appManagement:IAppManagement, httpServerPrexix="") {
        super(appManagement);
        let data: FlowchartData = {operators:[],links:[]};
        let options = new FlowchartOptions(httpServerPrexix);
        let callbacks = new FlowchartCallback();
        this.fc = new Flowchart(this.appManagement, data, callbacks, options);
        this.appManagement.RegisterNamespace(this, functionblock.NAMESPACE_ID);
    }
}
