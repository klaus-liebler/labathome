import { ConnectorType, FlowchartInputConnector, FlowchartOutputConnector } from "./FlowchartConnector";
import { FlowchartCompiler, HashAndBufAndMaps } from "./FlowchartCompiler";
import { FlowchartLink } from "./FlowchartLink";
import { FlowchartOperator, TypeInfo } from "./FlowchartOperator";
import * as operatorimpl from "./FlowchartOperatorImpl";
import { ColorNumColor2ColorDomString, EventCoordinatesInSVG, Html, Svg } from "@klaus-liebler/web-components/typescript/utils/common";
import { IAppManagement } from "@klaus-liebler/web-components/typescript/utils/interfaces";
import { SimulationManager } from "./SimulationManager";
import { FlowchartData, OperatorData, LinkData } from "./FlowchartData";
import { FilelistDialog, FilenameDialog, OkDialog } from "@klaus-liebler/web-components/typescript/dialog_controller";
import { functionblock } from "@generated/wsprotocol_ts/ws-protocol";
import { Menu, MenuItem, MenuManager } from "./MenuManager";
import { KeyValueTuple, Severity } from "@klaus-liebler/commons";
import "@klaus-liebler/web-components/style/flowchart.css"

//see devicemanager.hh
const FBDSTORE_BASE_DIRECTORY = "/spiffs/fbdstore/";    
const DEFAULTFBD_FBD_FILEPATH =  "/spiffs/defaultfbd.fbd";
const TEMPFBD_FBD_FILEPATH = "/spiffs/tempfbd.fbd";

export class FlowchartOptions {
    canUserEditLinks: boolean = true;
    canUserMoveOperators: boolean = true;
    distanceFromArrow: number = 3;
    defaultOperatorClass: string = 'flowchart-default-operator';
    defaultLinkColor: string = '#3366ff';
    defaultSelectedLinkColor: string = 'black';
    linkWidth: number = 10;
    grid: number = 10;
    multipleLinksOnOutput: boolean = true;
    multipleLinksOnInput: boolean = false;
    linkVerticalDecal: number = 0;
    httpServerBasePath="/files"
    constructor(httpServerPrexix:string){
        this.httpServerBasePath=httpServerPrexix+this.httpServerBasePath;
    }
}

export class FlowchartCallback {
    onOperatorSelect?: (operatorId: string) => boolean;
    onOperatorUnselect?: () => boolean;
    onOperatorMouseOver?: (operatorId: string) => boolean;
    onOperatorMouseOut?: (operatorId: string) => boolean;
    onLinkSelect?: (link: FlowchartLink) => boolean;
    onLinkUnselect?: (link: FlowchartLink) => boolean;
    onOperatorCreate?: (operatorId: string, operatorData: any, fullElement: boolean) => boolean;
    onLinkCreate?: (linkId: string, linkData: any) => boolean;
    onOperatorDelete?: (operatorId: string) => boolean;
    onLinkDelete?: (linkId: string, forced: boolean) => boolean;
    onOperatorMoved?: (operatorId: string, position: number) => void;
    onAfterChange?: (changeType: any) => void;
}

enum FlowchartMode{
    EDIT,
    SIMULATE,
    DEBUG,
}

export class Flowchart {
    UserMayMoveOperators() {
        return this.mode==FlowchartMode.EDIT && this.options.canUserMoveOperators;
    }
    
    TriggerDebug() {
        if(this.mode!=FlowchartMode.DEBUG) return;
        this.appManagement.SendFrame(functionblock.NAMESPACE_ID, functionblock.RequestDebugData.encode({requestId:0}));
    }
    OnMessage(namespaceId: number, messageTypeId: number, view: DataView) {
        if(namespaceId!=functionblock.NAMESPACE_ID) return;
        switch (messageTypeId) {
            case functionblock.ResponseDebugData.TYPE_ID:
                this.onResponseDebugData(functionblock.ResponseDebugData.decode(view, 0));
                break
            case functionblock.ResponseFbdRun.TYPE_ID:
                this.onResponseFbdRun(functionblock.ResponseFbdRun.decode(view, 0))
                break
        }
    }

    private mode=FlowchartMode.EDIT;
    private operatorRegistry: operatorimpl.OperatorRegistry;
    private simulationManager?: SimulationManager | null;
    private operators = new Map<number, FlowchartOperator>();
    private links = new Map<number, FlowchartLink>();
    public static readonly DATATYPE2COLOR = new Map([[ConnectorType.BOOLEAN, "RED"], [ConnectorType.COLOR, "GREEN"], [ConnectorType.FLOAT, "BLUE"], [ConnectorType.INTEGER, "YELLOW"], [ConnectorType.COLOR, "PURPLE"]]);
    //Muss beim Löschen+Erzeugen von Operatoren+Links und bei Speichern von Properties zurückgesetzt werden
    private currentDebugInfo: HashAndBufAndMaps | null = null;
    private lastOutputConnectorClicked: FlowchartOutputConnector | null = null;
    private selectedOperator: FlowchartOperator | null = null;
    private selectedLink: FlowchartLink | null = null;
    get SelectedLink() { return this.selectedLink };
    get Options() { return this.options; }

    private positionRatio: number = 1;
    get PositionRatio() { return this.positionRatio; }

    private flowchartContainerSvgSvg!: SVGSVGElement;
    get Element() { return this.flowchartContainerSvgSvg; }
    private linksLayer!: SVGGElement;
    get LinkLayer() { return this.linksLayer; }
    private operatorsLayer!: SVGGElement;
    get OperatorsLayer() { return this.operatorsLayer; }
    private operatorLibDiv!: HTMLDivElement;
    get ToolsLayer() { return this.operatorLibDiv; }
    private tempLayer!: SVGGElement;
    private temporaryLink!: SVGLineElement;
    private temporaryLinkSnapped = false;
    private propertyGridHtmlDiv!: HTMLDivElement;

    private markerArrow: SVGPathElement|null=null;
    private markerCircle: SVGCircleElement|null=null;

    private onResponseDebugData(d: functionblock.ResponseDebugData.Payload) {
        
        console.info(`Received debug data`);
        
        if(this.mode!=FlowchartMode.DEBUG){
            console.warn(`this.mode!=FlowchartMode.DEBUG, is ${FlowchartMode[this.mode]}`)
            return;
        }
        if (this.currentDebugInfo == null){
            console.warn(`this.currentDebugInfo == null`)
            return;
        }

        if (d.debugInfoHash != this.currentDebugInfo.hash) {
            console.error(`${d.debugInfoHash} != ${this.currentDebugInfo.hash}`);
            this.currentDebugInfo = null;
            return;
        }
        for (let adressOffset = 0; adressOffset < d.bools.length; adressOffset++) {
            var value = d.bools[adressOffset].value;
            if (adressOffset < 2) continue;

            let connectorType = ConnectorType.BOOLEAN
            let map = this.currentDebugInfo.typeIndex2adressOffset2ListOfLinks.get(connectorType)!;
            let linksToChange = map.get(adressOffset);
            if (linksToChange === undefined) {
                console.error(`linksToColorize===undefined for connectorType ${connectorType} addressOffset ${adressOffset} and value ${value}`);
                continue;
            }
            linksToChange.forEach((e) => {
                e.SetColor(value ? "red" : "grey");
                e.SetCaption("" + value);
            });
        }

        for (let adressOffset = 0; adressOffset < d.integers.length; adressOffset++) {
            let value = d.integers[adressOffset].value;
            if (adressOffset < 2) continue;
            let connectorType = ConnectorType.INTEGER
            let map = this.currentDebugInfo.typeIndex2adressOffset2ListOfLinks.get(connectorType)!;
            let linksToChange = map.get(adressOffset);
            if (linksToChange === undefined) {
                console.error(`linksToColorize===undefined for connectorType ${connectorType} addressOffset ${adressOffset} and value ${value}`);
                continue;
            }
            linksToChange.forEach((e) => {
                e.SetCaption(`${value}`);
            });
        }

        for (let adressOffset = 0; adressOffset < d.floats.length; adressOffset++) {
            let value = d.floats[adressOffset].value
            if (adressOffset < 2) continue;
            let connectorType = ConnectorType.FLOAT
            let map = this.currentDebugInfo.typeIndex2adressOffset2ListOfLinks.get(connectorType)!;
            let linksToChange = map.get(adressOffset);
            if (linksToChange === undefined) {
                console.error(`linksToColorize===undefined for connectorType ${connectorType} addressOffset ${adressOffset} and value ${value}`);
                continue;
            }
            linksToChange.forEach((e) => {
                e.SetCaption(value!.toFixed(2));
            });
        }

        for (let adressOffset = 0; adressOffset < d.colors.length; adressOffset++) {
            let value = d.colors[adressOffset].value
            if (adressOffset < 2) continue;
            let connectorType = ConnectorType.COLOR
            let map = this.currentDebugInfo.typeIndex2adressOffset2ListOfLinks.get(connectorType)!;
            let linksToChange = map.get(adressOffset);
            if (linksToChange === undefined) {
                console.error(`linksToColorize===undefined for connectorType ${connectorType} addressOffset ${adressOffset} and value ${value}`);
                continue;
            }
            linksToChange.forEach((e) => {
                e.SetCaption("" + value);
                e.SetColor(ColorNumColor2ColorDomString(value!));
            });
        }
    }

    public _notifyGlobalMousemoveWithLink(e: MouseEvent) {
        if(this.mode!=FlowchartMode.EDIT) return;
        if (this.lastOutputConnectorClicked != null && !this.temporaryLinkSnapped) {
            let end = EventCoordinatesInSVG(e, this.flowchartContainerSvgSvg, this.positionRatio);
            this.temporaryLink.setAttribute('x2', "" + end.x);
            this.temporaryLink.setAttribute('y2', "" + end.y);
        }
    }

    public _notifyGlobalMouseupWithLink(_e: MouseEvent) {
        if(this.mode!=FlowchartMode.EDIT) return;
        this.unsetTemporaryLink();
    }

    public _notifyOutputConnectorMousedown(c: FlowchartOutputConnector, e: MouseEvent) {
        if(this.mode!=FlowchartMode.EDIT) return;
        this.temporaryLinkSnapped = false;
        let start = c.GetLinkpoint();
        let end = EventCoordinatesInSVG(e, this.flowchartContainerSvgSvg, this.positionRatio);
        this.temporaryLink.setAttribute('x1', "" + start.x);
        this.temporaryLink.setAttribute('y1', "" + start.y);
        this.temporaryLink.setAttribute('x2', "" + end.x);
        this.temporaryLink.setAttribute('y2', "" + end.y);
        this.setTemporaryLink(c);
        document.onmouseup = (e) => {
            document.onmouseup = null;
            document.onmousemove = null;
            this._notifyGlobalMouseupWithLink(e);
        };
        document.onmousemove = (e) => {
            this._notifyGlobalMousemoveWithLink(e);
        };
    }

    public _notifyInputConnectorMouseup(c: FlowchartInputConnector, _e: MouseEvent) {
        if(this.mode!=FlowchartMode.EDIT) return;
        if (this.lastOutputConnectorClicked == null) return;
        if (!this.options.multipleLinksOnInput && c.LinksLength > 0) return;
        if (this.lastOutputConnectorClicked.Type == c.Type) {
            this.createLink(null, this.lastOutputConnectorClicked, c);
        }
        this.unsetTemporaryLink();

    }

    public _notifyOperatorClicked(o: FlowchartOperator, _e: MouseEvent) {
        if(this.mode!=FlowchartMode.EDIT) return;
        this.SelectOperator(o);
    }

    public _notifyLinkClicked(link: FlowchartLink, _e: MouseEvent) {
        if(this.mode!=FlowchartMode.EDIT) return;
        this.selectLink(link);
    }

    public _notifyInputConnectorMouseenter(c: FlowchartInputConnector, _e: MouseEvent) {
        if(this.mode!=FlowchartMode.EDIT) return;
        if (this.lastOutputConnectorClicked == null || this.lastOutputConnectorClicked.Type != c.Type) return;
        if (!this.options.multipleLinksOnInput && c.LinksLength > 0) return;

        this.temporaryLinkSnapped = true;
        let end = c.GetLinkpoint();
        this.temporaryLink.setAttribute("marker-end", "url(#marker-circle)");
        this.temporaryLink.setAttribute('x2', "" + end.x);
        this.temporaryLink.setAttribute('y2', "" + end.y);
    }

    public _notifyInputConnectorMouseleave(_c: FlowchartInputConnector, _e: MouseEvent) {
        if(this.mode!=FlowchartMode.EDIT) return;
        this.temporaryLinkSnapped = false;
        this.temporaryLink.setAttribute("marker-end", "url(#marker-arrow)");
    }

    public unselectLink() {
        if(this.mode!=FlowchartMode.EDIT) return;
        if (this.selectedLink != null) {
            if (this.flowchartCallbacks.onLinkUnselect && !this.flowchartCallbacks.onLinkUnselect(this.selectedLink)) {
                return;
            }
            this.selectedLink.UnsetColor();
            this.selectedLink = null;
        }
    }

    public selectLink(link: FlowchartLink) {
        if(this.mode!=FlowchartMode.EDIT) return;
        this.unselectLink();
        if (this.flowchartCallbacks.onLinkSelect && !this.flowchartCallbacks.onLinkSelect(link)) {
            return;
        }
        this.unselectOperator();
        this.selectedLink = link;
        link.SetColor(this.options.defaultSelectedLinkColor);
    }

    private deleteSelectedThing(): void {
        if(this.mode!=FlowchartMode.EDIT) return;
        if (this.selectedOperator) {
            this.DeleteOperator(this.selectedOperator.GlobalOperatorIndex);
        }
        else if (this.selectedLink) {
            this.DeleteLink(this.selectedLink.GlobalLinkIndex);
        }
    }

    private createFlowchartDataJSONString(): string {
        let operators: OperatorData[] = [];
        let links: LinkData[] = [];
        for (const op of this.operators.values()) {
            operators.push({ globalTypeIndex: op.TypeInfo.GlobalTypeIndex, caption: op.Caption, index: op.GlobalOperatorIndex, posX: op.Xpos, posY: op.Ypos, configurationData: op.Config_Copy });
        }
        for (const link of this.links.values()) {
            links.push({
                fromOperatorIndex: link.From.Parent.GlobalOperatorIndex,
                fromOutput: link.From.LocalConnectorIndex,
                toOperatorIndex: link.To.Parent.GlobalOperatorIndex,
                toInput: link.To.LocalConnectorIndex,
            });
        }
        return JSON.stringify({ operators: operators, links: links });
    }

    private createFbdFile(){
        //Die Datei besteht aus
        //4 Bytes mit der Länge des Binärteils
        //Dem Binärteil
        //Dem JSON-Teil, der die grafische Darstellung enthält
        var compilerInstance = new FlowchartCompiler(this.operators);
        var guidAndBufAndMap = compilerInstance.Compile();
        this.currentDebugInfo=guidAndBufAndMap;
        var viewByteLength = new DataView(new ArrayBuffer(4));
        viewByteLength.setUint32(0, guidAndBufAndMap.buf.byteLength, true); //ESP32 is little-endian: true für little-endian, false für big-endian
        var stringBuffer = new TextEncoder().encode(this.createFlowchartDataJSONString()).buffer;
        var combinedBuffer = new Uint8Array(viewByteLength.byteLength + guidAndBufAndMap.buf.byteLength + stringBuffer.byteLength);
        combinedBuffer.set(new Uint8Array(viewByteLength.buffer), 0);
        combinedBuffer.set(new Uint8Array(guidAndBufAndMap.buf), 4);
        combinedBuffer.set(new Uint8Array(stringBuffer), 4 + guidAndBufAndMap.buf.byteLength);
        return combinedBuffer 
    }

    private parseFbdFile(arrayBuffer:ArrayBuffer):FlowchartData {
        const dataView = new DataView(arrayBuffer);
        // Lesen Sie die Länge des Binärteils (erste 4 Bytes)
        const binaryLength = dataView.getUint32(0, true);
        // Extrahieren Sie den Binär- und den JSON-Teil
        //const _binaryPart = new Uint8Array(arrayBuffer, 4, binaryLength);
        const jsonPart = new TextDecoder().decode(arrayBuffer.slice(4 + binaryLength));
        return JSON.parse(jsonPart);
    }

    private saveFbdToLocalFile() {
        let blob = new Blob([this.createFbdFile()], { type: "octet/stream" });
        let url = window.URL.createObjectURL(blob);
        let filename = "functionBlockDiagram.fbd";
        var element = document.createElement('a');
        element.style.display = 'none';
        element.href = url;
        element.download = filename;
        document.body.appendChild(element);
        element.click();
        document.body.removeChild(element);
    }

    private openFbdFromLocalFile(files: FileList | null) {
        if (files == null || files.length != 1) return;
        const reader = new FileReader();
        reader.onloadend = (e) => {
            let buf: ArrayBuffer = <ArrayBuffer>e.target!.result;
            this.setData(this.parseFbdFile(buf));
        }
        reader.readAsArrayBuffer(files[0]);
    }

    private onResponseFbdRun(_m:functionblock.ResponseFbdRun.Payload){
        this.appManagement.ShowSnackbar(Severity.SUCCESS,`File now runs on Lab@Home`);
    }

    private async postFbdFile(path:string, onSuccessAction?:(path:string)=>void, onFailAction?:(path:string)=>void){

        try {
            const response = await fetch(this.options.httpServerBasePath + path, {
                method: 'POST',
                body: this.createFbdFile(),
                headers: {
                'Content-Type': 'application/octet-stream'
                }
            });

            if (!response.ok) {
                this.appManagement.ShowDialog(new OkDialog(Severity.ERROR, `HTTP Error ${response.status}`));
                if (onFailAction) onFailAction(path);
                return;
            }

            this.appManagement.ShowSnackbar(Severity.SUCCESS, `Successfully saved`);
            if (onSuccessAction) onSuccessAction(path);

        } catch (error) {
            console.error('There was a problem with the post operation:', error);
            this.appManagement.ShowDialog(new OkDialog(Severity.ERROR, `Generic Error`));
            if (onFailAction) onFailAction(path);
        }
    }

    private async getFbdFile(path: string) {
        try {
            const response = await fetch(this.options.httpServerBasePath + path);
            if (!response.ok) {
                throw new Error(`Failed to load file from path:path:${response.statusText}`);
            }
            const arrayBuffer = await response.arrayBuffer();
            if (arrayBuffer.byteLength === 0) {
                console.error(`Error loading file from ${path}: File has size 0`);
                return;
            }
            this.setData(this.parseFbdFile(arrayBuffer))
        } catch (error) {
            this.appManagement.ShowDialog(new OkDialog(Severity.ERROR, `Failed to load file from ${path}`));
            console.error(`Error loading file from ${path}:`, error);
        }
    }

    private async deleteFdbFile(path: string) {
        try {
            const response = await fetch(this.options.httpServerBasePath + path, {
                method: 'DELETE',
            });
    
            if (!response.ok) {
                throw new Error(`Failed to delete file, status: ${response.status}`);
            }
    
            this.appManagement.ShowSnackbar(Severity.SUCCESS, `File ${path} deleted successfully`);
        } catch (error: any) {
            console.error('There was a problem with the delete operation:', error);
            this.appManagement.ShowSnackbar(Severity.ERROR, `Failed to delete file ${path}: ${error.message}`);
        }
    }

    private async getFbdFileList(path_with_slash_at_the_end:string) {
        try {
            const response = await fetch(this.options.httpServerBasePath + path_with_slash_at_the_end);
            
            if (!response.ok) {
                throw new Error(`Network response was not ok, status: ${response.status}`);
            }
    
            const data = await response.json();
    
            if (!data.files || !data.dirs) {
                throw new Error('Response format is incorrect');
            }
    
            this.appManagement.ShowDialog(new FilelistDialog((<string[]>data.files).filter(v=>v.endsWith(".fbd")),
                (ok, filename) => {
                    if (!ok) return;
                    this.getFbdFile(path_with_slash_at_the_end + filename);
                },
                (ok, filename) => {
                    if (!ok) return;
                    this.deleteFdbFile(path_with_slash_at_the_end + filename);
                }
            ));
        } catch (error) {
            console.error('There was a problem with the fetch operation:', error);
            // Optionally, provide user feedback about the error
        }
    }

    private enterFilenameAndPostFbd(){
        this.appManagement.ShowDialog(new FilenameDialog("Enter filename (without Extension", (ok:boolean, filename:string)=>{
            if(!ok) return
            this.postFbdFile(FBDSTORE_BASE_DIRECTORY+filename+".fbd")
        }));

    }
    
    private buildMenu(subcontainer: HTMLDivElement) {
        let fileInput = <HTMLInputElement>Html(subcontainer, "input", ["type", "file", "id", "fileInput", "accept", ".json"]);
        fileInput.style.display = "none";
        fileInput.onchange = (_e) => {
            this.openFbdFromLocalFile(fileInput.files);
        }
        var mm:MenuManager=new MenuManager(
            [
                new Menu("File", [
                    new MenuItem("📂 Open (Local)", ()=>fileInput.click()),
                    new MenuItem("📂 Open (labathome)", ()=>this.getFbdFileList(FBDSTORE_BASE_DIRECTORY)),
                    new MenuItem("📂 Open Default (labathome)", ()=>this.getFbdFile(DEFAULTFBD_FBD_FILEPATH)),
                    new MenuItem("💾 Save (Local)", ()=>this.saveFbdToLocalFile()),
                    new MenuItem("💾 Save (labathome)", ()=>this.enterFilenameAndPostFbd()),
                ]),
                new Menu("Debug",[
                    new MenuItem("☭ Start Debug", ()=>this.postFbdFile(TEMPFBD_FBD_FILEPATH, 
                        (_p:string)=>{

                            this.appManagement.SendFrame(functionblock.NAMESPACE_ID, functionblock.RequestFbdRun.encode({requestId:0}), 3000);
                            this.mode=FlowchartMode.DEBUG;
                            this.flowchartContainerSvgSvg.classList.remove("edit", "simulate");
                            this.flowchartContainerSvgSvg.classList.add("debug");
                        },
                        (p:string)=>{
                           console.error(`As file "${p}" could no be saved on labathome, the RequestFbdRun will not be sent to labathome`)
                        }
                    )),
                    new MenuItem("× Stop Debug", ()=>{
                        this.mode=FlowchartMode.EDIT;
                        this.flowchartContainerSvgSvg.classList.remove("simulate", "debug");
                        this.flowchartContainerSvgSvg.classList.add("edit");
                        this.resetColorsAndCaptions();
                    }), 
                    new MenuItem("👣 Set as Startup-App", ()=>this.postFbdFile(DEFAULTFBD_FBD_FILEPATH)), 
                ]),
                new Menu("Simulation",[
                    new MenuItem("➤ Start Simulation", ()=>{
                        let compilerInstance = new FlowchartCompiler(this.operators);
                        this.simulationManager = new SimulationManager(compilerInstance.CompileForSimulation());
                        this.simulationManager.Start(false);
                        this.mode=FlowchartMode.SIMULATE;
                        this.flowchartContainerSvgSvg.classList.remove("edit", "debug");
                        this.flowchartContainerSvgSvg.classList.add("simulate");
                    }),
                    new MenuItem("× Stop Simulation",()=>{
                        this.simulationManager?.Stop();
                        this.mode=FlowchartMode.EDIT;
                        this.resetColorsAndCaptions();
                        this.flowchartContainerSvgSvg.classList.remove("simulate", "debug");
                        this.flowchartContainerSvgSvg.classList.add("edit");
                    }) 
                ])
            ]
        );
        mm.Render(subcontainer)
    }
    resetColorsAndCaptions() {
        this.operators.forEach(o=>o.ResetColorsAndCaptions());
        this.links.forEach(l=>l.SetCaption(""))
        this.links.forEach(l=>l.UnsetColor())
    }

    public RenderUi(subcontainer: HTMLDivElement) {
        if (!subcontainer) throw new Error("container is null");
        //let subcontainer = <HTMLDivElement>Html(container, "div", [], ["develop-ui"]);
        

        this.buildMenu(subcontainer);

        let workspace = <HTMLDivElement>Html(subcontainer, "div", ["tabindex", "0"], ["develop-workspace"]);//tabindex, damit keypress-Events abgefangen werden können
        this.propertyGridHtmlDiv = <HTMLDivElement>Html(subcontainer, "div", [], ["develop-properties"]);

        this.flowchartContainerSvgSvg = <SVGSVGElement>Svg(workspace, "svg", ["width", "100%", "height", "100%"], ["flowchart-container", "edit"]);

        this.linksLayer = <SVGGElement>Svg(this.flowchartContainerSvgSvg, "g", [], ["flowchart-links-layer"]);
        this.operatorsLayer = <SVGGElement>Svg(this.flowchartContainerSvgSvg, "g", [], ["flowchart-operators-layer", "unselectable"]);
        this.tempLayer = <SVGSVGElement>Svg(this.flowchartContainerSvgSvg, "g", [], ["flowchart-temporary-link-layer"]);
        this.tempLayer.style.visibility = "hidden";//visible
        let defs = Svg(this.tempLayer, "defs", []);
        let markerArrow = Svg(defs, "marker", ["id", "marker-arrow", "markerWidth", "4", "markerHeight", "4", "refX", "1", "refY", "2", "orient", "0"]);
        this.markerArrow = <SVGPathElement>Svg(markerArrow, "path", ["d", "M0,0 L0,4 L2,2 z", "fill", "red", "stroke", "black", "stroke-width", "0.5"]);
        let markerCircle = Svg(defs, "marker", ["id", "marker-circle", "markerWidth", "4", "markerHeight", "4", "refX", "2", "refY", "2", "orient", "0"]);
        this.markerCircle = <SVGCircleElement>Svg(markerCircle, "circle", ["cx", "2", "cy", "2", "r", "2", "fill", "red", "stroke-width", "1px", "stroke", "black"]);
        this.temporaryLink = <SVGLineElement>Svg(this.tempLayer, "line", ["x1", "0", "y1", "0", "x2", "0", "y2", "0", "stroke-dasharray", "6,6", "stroke-width", "4", "stroke", "black", "fill", "none", "marker-end", "url(#marker-arrow)"]);

        let operatorLibActivator = <SVGRectElement>Svg(this.flowchartContainerSvgSvg, "rect", ["width", "40", "height", "100%", "fill", "white", "fill-opacity", "0"]);

        this.operatorLibDiv = <HTMLDivElement>Html(workspace, "div", [], ["flowchart-operatorlibdiv", "unselectable"]);
        this.operatorLibDiv.style.display = "none";


        //let toolsRect= <SVGRectElement>$.Svg(this.operatorLibDiv, "rect", ["width","140", "height", "100%", "rx", "10", "ry", "10"], ["tools-container"]);

        //The onmousemove event occurs every time the mouse pointer is moved over the div element.
        //The mouseenter event only occurs when the mouse pointer enters the div element.
        //The onmouseover event occurs when the mouse pointer enters the div element, and its child elements (p and span).

        //The mouseout event triggers when the mouse pointer leaves any child elements as well the selected element.
        //The mouseleave event is only triggered when the mouse pointer leaves the selected element.
        operatorLibActivator.onmouseenter = (_e) => {
            this.operatorLibDiv.style.display = "inline";
        }
        this.operatorLibDiv.onmouseleave = (_e) => {
            this.operatorLibDiv.style.display = "none";
        }

        this.flowchartContainerSvgSvg.onclick = (e) => {
            if (e.target == this.Element)//if the click is in a "free" area, then the target is the uppermost layer; the linkLayer!
            {
                this.unselectOperator();
                this.unselectLink();
            }
        }

        workspace.onkeyup = (e) => {
            if (e.key == "Delete") {
                console.debug("Flowchart workspace.onkeyup with e.target=" + e.target + " und Delete-Key");
                this.deleteSelectedThing();
            }
            else {
                console.debug("Flowchart workspace.onkeyup with e.target=" + e.target + " und key " + e.key);
            }
        }

        this.operatorRegistry.populateOperatorLib(this.operatorLibDiv, (e: MouseEvent, ti: TypeInfo) => {
            let caption = ti.OperatorName;
            let o = this.createOperatorInternal(ti.GlobalTypeIndex, caption, null);
            let coords = EventCoordinatesInSVG(e, this.Element);
            o.MoveTo(coords.x - 10, coords.y - 10);
            o.RegisterDragging(e);
            this.operators.set(o.GlobalOperatorIndex, o);
        });

        this.getFbdFile(DEFAULTFBD_FBD_FILEPATH);
        this.recreateFlowchartFromData();
    }

    constructor(private appManagement: IAppManagement, private flowchartData: FlowchartData, private flowchartCallbacks: FlowchartCallback, private options: FlowchartOptions) {
        this.operatorRegistry = operatorimpl.OperatorRegistry.Build();
    }


    private createOperatorInternal(globalTypeIndex: number, caption: string, configurationData: KeyValueTuple[] | null): FlowchartOperator {

        if (!this.operatorRegistry.IsIndexKnown(globalTypeIndex)) {
            throw new Error(`Unknown globalTypeIndex ${globalTypeIndex}`);
        }
        if (this.flowchartCallbacks.onOperatorCreate && !this.flowchartCallbacks.onOperatorCreate(caption, null, false)) {
            throw new Error(`Creation of operator of globalTypeIndex ${globalTypeIndex} prevented by onOperatorCreate plugin`);
        }
        let op = this.operatorRegistry.CreateByIndex(globalTypeIndex, this, caption, configurationData)!;

        this.currentDebugInfo = null;
        return op;
    }

    public setData(data: FlowchartData) {
        this.flowchartData = data;
        this.recreateFlowchartFromData();

    }

    private recreateFlowchartFromData() {
        this.links.forEach((e) => e.RemoveFromDOM());
        this.links.clear();
        this.operators.forEach((e) => e.RemoveFromDOM());
        this.operators.clear();
        FlowchartOperator.ResetMaxIndex();
        let indexInData2operator = new Map<number, FlowchartOperator>();

        for (const d of this.flowchartData!.operators) {
            let o = this.createOperatorInternal(d.globalTypeIndex, d.caption, d.configurationData);
            o.MoveTo(d.posX, d.posY);
            this.operators.set(o.GlobalOperatorIndex, o);
            indexInData2operator.set(d.index, o);
        }
        for (const d of this.flowchartData!.links) {
            let fromOp = indexInData2operator.get(d.fromOperatorIndex);
            let toOp = indexInData2operator.get(d.toOperatorIndex);
            if (fromOp === undefined || toOp === undefined) continue;
            let fromConn = fromOp.GetOutputConnectorByIndex(d.fromOutput);
            let toConn = toOp.GetInputConnectorByIndex(d.toInput);
            if (fromConn == null || toConn == null) continue;
            this.createLink(d, fromConn, toConn);
        }
    }

    public DeleteLink(globalLinkIndex: number) {
        this.currentDebugInfo = null;
        let l = this.links.get(globalLinkIndex);
        if (l === undefined) {
            throw Error("Link to delete is undefined")
        }
        if (this.selectedLink == l) {
            this.unselectLink();
        }
        l.RemoveFromDOM();
        this.links.delete(globalLinkIndex);
        l.To.RemoveLink(l);
        l.From.RemoveLink(l);
    }

    public DeleteOperator(globalOperatorIndex: number) {
        this.currentDebugInfo = null;
        let o = this.operators.get(globalOperatorIndex);
        if (o === undefined) {
            throw Error("Operator to delete is undefined")
        }
        if (this.selectedOperator == o) {
            this.unselectOperator();
        }
        o.RemoveFromDOM();
        this.operators.delete(o.GlobalOperatorIndex);
        for (const outputKV of o.OutputsKVIt) {
            for (const linkKV of outputKV[1].LinksKVIt) {
                this.DeleteLink(linkKV[1].GlobalLinkIndex);
            }
        }
        for (const inputKV of o.InputsKVIt) {
            for (const linkKV of inputKV[1].LinksKVIt) {
                this.DeleteLink(linkKV[1].GlobalLinkIndex);
            }
        }
    }

    public createLink(data: LinkData | null, from: FlowchartOutputConnector, to: FlowchartInputConnector): FlowchartLink | null {
        if (this.flowchartCallbacks.onLinkCreate && !this.flowchartCallbacks.onLinkCreate(from.Caption, data)) return null;
        if (!this.options.multipleLinksOnOutput && from.LinksLength > 0) return null;
        if (!this.options.multipleLinksOnInput && to.LinksLength > 0) return null;
        this.currentDebugInfo = null;
        let l: FlowchartLink = new FlowchartLink(this, "", this.Options.defaultLinkColor, from, to);
        from.AddLink(l);
        to.AddLink(l);
        this.links.set(l.GlobalLinkIndex, l);
        return l;
    }

    private unsetTemporaryLink() {
        this.lastOutputConnectorClicked = null;
        this.tempLayer.style.visibility = "hidden";
    }

    private setTemporaryLink(c: FlowchartOutputConnector) {
        this.lastOutputConnectorClicked = c;
        let color = Flowchart.DATATYPE2COLOR.get(c.Type)
        if (!color) color = "BLACK";
        this.markerArrow!.style.fill = color;
        this.markerCircle!.style.fill = color;
        this.tempLayer.style.visibility = "visible";
    }

    private unselectOperator() {
        if (this.flowchartCallbacks.onOperatorUnselect && !this.flowchartCallbacks.onOperatorUnselect()) return;
        this.propertyGridHtmlDiv.innerText = ""; //clear
        if (this.selectedOperator == null) return;
        this.selectedOperator.ShowAsSelected(false);
        this.selectedOperator = null;
    }

    public SelectOperator(operator: FlowchartOperator): void {
        if (this.flowchartCallbacks.onOperatorSelect && !this.flowchartCallbacks.onOperatorSelect(operator.Caption)) return;
        this.unselectLink();
        if (this.selectedOperator != null) this.selectedOperator.ShowAsSelected(false);
        operator.ShowAsSelected(true);
        this.selectedOperator = operator;
        this.propertyGridHtmlDiv.innerText = ""; //clear
        Html(this.propertyGridHtmlDiv, "p", [], ["develop-propertygrid-head"], `Properties for ${this.selectedOperator.Caption}`);
        let table = <HTMLTableElement>Html(this.propertyGridHtmlDiv, "table", [], ["develop-propertygrid-table"]);
        let thead = <HTMLTableSectionElement>Html(table, "thead", [], []);
        let tr_head = Html(thead, "tr", [], ["develop-propertygrid-tr"]);
        Html(tr_head, "th", [], ["develop-propertygrid-th"], "Key");
        Html(tr_head, "th", [], ["develop-propertygrid-th"], "Value");
        let tbody = <HTMLTableSectionElement>Html(table, "tbody", [], []);
        if (this.selectedOperator!.PopulateProperyGrid(tbody)) {
            Html(this.propertyGridHtmlDiv, "button", [], ["develop-propertygrid-button"], `Save`).onclick = (_e) => {
                this.currentDebugInfo = null;
                operator.SavePropertyGrid(tbody);
            };
        }
        else {
            this.propertyGridHtmlDiv.innerText = ""; //clear
            Html(this.propertyGridHtmlDiv, "p", [], ["develop-propertygrid-head"], `No Properties for ${this.selectedOperator.Caption}`);
        }
    }

    // Found here : http://stackoverflow.com/questions/5560248/programmatically-lighten-or-darken-a-hex-color-or-rgb-and-blend-colors
    public static _shadeColor(color: string, percent: number) {
        var f = parseInt(color.slice(1), 16), t = percent < 0 ? 0 : 255, p = percent < 0 ? percent * -1 : percent, R = f >> 16, G = f >> 8 & 0x00FF, B = f & 0x0000FF;
        return "#" + (0x1000000 + (Math.round((t - R) * p) + R) * 0x10000 + (Math.round((t - G) * p) + G) * 0x100 + (Math.round((t - B) * p) + B)).toString(16).slice(1);
    }
}
