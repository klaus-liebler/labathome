import { html } from "lit-html";
import { Flowchart, FlowchartCallback, FlowchartOptions } from "../flowchart/Flowchart";
import { FlowchartData } from "../flowchart/FlowchartData";
import { IAppManagement } from "../utils/interfaces";
import { ScreenController } from "./screen_controller";
import { createRef, ref, Ref } from "lit-html/directives/ref.js";
import { ByteBuffer } from "flatbuffers";


import { Namespace, ResponseDebugData, Responses, ResponseWrapper} from "../../generated/flatbuffers/functionblock";


export class DevelopCFCController extends ScreenController {
    
    OnMessage(namespace: number, bb: ByteBuffer): void {
        if(namespace!=Namespace.Value) return;
        let messageWrapper = ResponseWrapper.getRootAsResponseWrapper(bb)
        switch (messageWrapper.responseType()) {
            case Responses.ResponseDebugData:
                this.fc.OnResponseDebugData(<ResponseDebugData>messageWrapper.response(new ResponseDebugData()));
        }
    }
    private mainDiv:Ref<HTMLInputElement> = createRef();
    private fc: Flowchart;
    private timer: number | undefined;

    public Template = () => html`<div ${ref(this.mainDiv)} class="develop-ui"></div>`
    

    OnFirstStart(): void {
        this.timer = window.setInterval(() => { this.fc.triggerDebug(); }, 1000);
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
        let data: FlowchartData = {
            operators: [
                {
                    index: 0,
                    caption: "RedButton",
                    globalTypeIndex: 34,
                    posX: 10,
                    posY: 10,
                    configurationData: null,
                },
                {
                    index: 1,
                    caption: "GreenButton",
                    globalTypeIndex: 31,
                    posX: 10,
                    posY: 150,
                    configurationData: null,
                },
                {
                    index: 2,
                    caption: "AND",
                    globalTypeIndex: 1,
                    posX: 250,
                    posY: 10,
                    configurationData: null,
                },
                {
                    index: 3,
                    caption: "RedLed",
                    globalTypeIndex: 50,
                    posX: 500,
                    posY: 10,
                    configurationData: null,
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
        let callbacks = new FlowchartCallback();
        this.fc = new Flowchart(this.appManagement, data, callbacks, options);
    }
}
