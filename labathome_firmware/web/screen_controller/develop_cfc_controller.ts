import { html } from "lit-html";
import { Flowchart, FlowchartCallback, FlowchartOptions, IFlowchartDevice } from "@klaus-liebler/flowchart";
import { IAppManagement } from "@klaus-liebler/web-components/typescript/utils/interfaces";
import { ScreenController } from "@klaus-liebler/web-components/typescript/screen_controller/screen_controller";
import { createRef, ref, Ref } from "lit-html/directives/ref.js";
import { functionblock } from "@generated/wsprotocol_ts/ws-protocol";

// Der Flowchart selbst (Editor + Simulator) liegt in npm-packages/@klaus-liebler/flowchart und wird
// auch standalone ohne Hardware genutzt (C:\repos\function_block_web). Dieser Controller ist die
// Geraeteanbindung: Datei-Endpunkt + ws-protocol-Namespace functionblock.
export class DevelopCFCController extends ScreenController implements IFlowchartDevice {

    public readonly Name = "labathome";
    public readonly HttpServerBasePath: string;
    private mainDiv:Ref<HTMLInputElement> = createRef();
    private fc: Flowchart;
    private timer: number | undefined;

    public Template = () => html`<div ${ref(this.mainDiv)} class="develop-ui"></div>`

    RequestFbdRun(): void {
        this.appManagement.SendFrame(functionblock.NAMESPACE_ID, functionblock.RequestFbdRun.encode({requestId:0}), 3000);
    }

    RequestDebugData(): void {
        this.appManagement.SendFrame(functionblock.NAMESPACE_ID, functionblock.RequestDebugData.encode({requestId:0}));
    }

    OnMessage(namespaceId: number, messageTypeId: number, view: DataView): void {
        if(namespaceId!=functionblock.NAMESPACE_ID) return;
        switch (messageTypeId) {
            case functionblock.ResponseDebugData.TYPE_ID: {
                const d = functionblock.ResponseDebugData.decode(view, 0);
                this.fc.OnDebugData({
                    debugInfoHash: d.debugInfoHash,
                    bools: d.bools.map(e => e.value),
                    integers: d.integers.map(e => e.value),
                    floats: d.floats.map(e => e.value),
                    colors: d.colors.map(e => e.value),
                });
                break;
            }
            case functionblock.ResponseFbdRun.TYPE_ID:
                this.fc.OnFbdRunConfirmed();
                break;
        }
    }

    OnFirstStart(): void {
        this.timer = window.setInterval(() => { this.fc.TriggerDebug();}, 1000);
        this.fc.RenderUi(this.mainDiv.value!);
    }
    OnRestart(): void {
        //UI existiert bereits -- nur den Debug-Timer wieder starten (RenderUi erneut aufzurufen wuerde Menue und Arbeitsflaeche duplizieren)
        this.timer = window.setInterval(() => { this.fc.TriggerDebug();}, 1000);
    }
    OnPause(): void {
        window.clearInterval(this.timer);
    }
    public OnCreate() { }

    constructor(appManagement:IAppManagement, httpServerPrexix="") {
        super(appManagement);
        this.HttpServerBasePath = httpServerPrexix + "/files";
        this.fc = new Flowchart(this.appManagement, {operators:[],links:[]}, new FlowchartCallback(), new FlowchartOptions(), this);
        this.appManagement.RegisterNamespace(this, functionblock.NAMESPACE_ID);
    }
}
