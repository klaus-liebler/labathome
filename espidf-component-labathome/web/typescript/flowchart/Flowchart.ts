import { ConnectorType, FlowchartInputConnector, FlowchartOutputConnector } from "./FlowchartConnector";
import { FlowchartCompiler, HashAndBufAndMaps } from "./FlowchartCompiler";
import { FlowchartLink } from "./FlowchartLink";
import { FlowchartOperator, TypeInfo } from "./FlowchartOperator";
import * as operatorimpl from "./FlowchartOperatorImpl";
import { ColorNumColor2ColorDomString, EventCoordinatesInSVG, Html, KeyValueTuple, Severity, Svg } from "../utils/common";
import { IAppManagement } from "../utils/interfaces";
import * as flatbuffers from 'flatbuffers';
import { SimulationManager } from "./SimulationManager";
import { FlowchartData, OperatorData, LinkData } from "./FlowchartData";
import { FilelistDialog, FilenameDialog, OkDialog } from "../dialog_controller/dialog_controller";
import { Namespace, RequestDebugData, RequestFbdRun, ResponseDebugData, ResponseFbdRun, Responses, ResponseWrapper } from "../../generated/flatbuffers/functionblock";
import { Menu, MenuItem, MenuManager } from "./MenuManager";

//see devicemanager.hh
const FBDSTORE_BASE_DIRECTORY = "/spiffs/fbdstore/";    
const DEFAULT_FBD_FILEPATH =  "/spiffs/default.fbd";
const TEMP_FBD_FILEPATH = "/spiffs/temp.fbd";

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
    TriggerDebug() {
        if(this.mode!=FlowchartMode.DEBUG) return;
        let b = new flatbuffers.Builder(1024);
        b.finish(RequestDebugData.createRequestDebugData(b))
        this.appManagement.WrapAndSend(Namespace.Value, b, 3000);
    }
    OnMessage(namespace: number, bb: flatbuffers.ByteBuffer) {
        if(namespace!=Namespace.Value) return;

        let messageWrapper = ResponseWrapper.getRootAsResponseWrapper(bb)
        switch (messageWrapper.responseType()) {
            case Responses.ResponseDebugData:
                this.onResponseDebugData(<ResponseDebugData>messageWrapper.response(new ResponseDebugData()));
                break
            case Responses.ResponseFbdRun:
                this.onResponseFbdRun(<ResponseFbdRun>messageWrapper.response(new ResponseFbdRun()))
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

    private flowchartContainerSvgSvg: SVGSVGElement;
    get Element() { return this.flowchartContainerSvgSvg; }
    private linksLayer: SVGGElement;
    get LinkLayer() { return this.linksLayer; }
    private operatorsLayer: SVGGElement;
    get OperatorsLayer() { return this.operatorsLayer; }
    private operatorLibDiv: HTMLDivElement;
    get ToolsLayer() { return this.operatorLibDiv; }
    private tempLayer: SVGGElement;
    private temporaryLink: SVGLineElement;
    private temporaryLinkSnapped = false;
    private propertyGridHtmlDiv: HTMLDivElement;

    private markerArrow: SVGPathElement;
    private markerCircle: SVGCircleElement;

    private onResponseDebugData(d: ResponseDebugData) {
        if(this.mode!=FlowchartMode.DEBUG) return;
        if (this.currentDebugInfo == null) return;

        if (d.debugInfoHash() != this.currentDebugInfo.hash) {
            console.error("hash!=this.currentDebugInfo.hash");
            this.currentDebugInfo = null;
            return;
        }
        for (let adressOffset = 0; adressOffset < d.boolsLength(); adressOffset++) {
            var value = d.bools(adressOffset);
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

        for (let adressOffset = 0; adressOffset < d.integersLength(); adressOffset++) {
            let value = d.integers(adressOffset);
            if (adressOffset < 2) continue;
            let connectorType = ConnectorType.INTEGER
            let map = this.currentDebugInfo.typeIndex2adressOffset2ListOfLinks.get(connectorType)!;
            let linksToChange = map.get(adressOffset);
            if (linksToChange === undefined) {
                console.error(`linksToColorize===undefined for connectorType ${connectorType} addressOffset ${adressOffset} and value ${value}`);
                continue;
            }
            linksToChange.forEach((e) => {
                e.SetCaption("" + value);
            });
        }

        for (let adressOffset = 0; adressOffset < d.floatsLength(); adressOffset++) {
            let value = d.floats(adressOffset)
            if (adressOffset < 2) continue;
            let connectorType = ConnectorType.FLOAT
            let map = this.currentDebugInfo.typeIndex2adressOffset2ListOfLinks.get(connectorType)!;
            let linksToChange = map.get(adressOffset);
            if (linksToChange === undefined) {
                console.error(`linksToColorize===undefined for connectorType ${connectorType} addressOffset ${adressOffset} and value ${value}`);
                continue;
            }
            linksToChange.forEach((e) => {
                e.SetCaption(value.toFixed(2));
            });
        }

        for (let adressOffset = 0; adressOffset < d.colorsLength(); adressOffset++) {
            let value = d.colors(adressOffset)
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
                e.SetColor(ColorNumColor2ColorDomString(value));
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

    public _notifyGlobalMouseupWithLink(e: MouseEvent) {
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

    public _notifyInputConnectorMouseup(c: FlowchartInputConnector, e: MouseEvent) {
        if(this.mode!=FlowchartMode.EDIT) return;
        if (this.lastOutputConnectorClicked == null) return;
        if (!this.options.multipleLinksOnInput && c.LinksLength > 0) return;
        if (this.lastOutputConnectorClicked.Type == c.Type) {
            this.createLink(null, this.lastOutputConnectorClicked, c);
        }
        this.unsetTemporaryLink();

    }

    public _notifyOperatorClicked(o: FlowchartOperator, e: MouseEvent) {
        if(this.mode!=FlowchartMode.EDIT) return;
        this.SelectOperator(o);
    }

    public _notifyLinkClicked(link: FlowchartLink, e: MouseEvent) {
        if(this.mode!=FlowchartMode.EDIT) return;
        this.selectLink(link);
    }

    public _notifyInputConnectorMouseenter(c: FlowchartInputConnector, e: MouseEvent) {
        if(this.mode!=FlowchartMode.EDIT) return;
        if (this.lastOutputConnectorClicked == null || this.lastOutputConnectorClicked.Type != c.Type) return;
        if (!this.options.multipleLinksOnInput && c.LinksLength > 0) return;

        this.temporaryLinkSnapped = true;
        let end = c.GetLinkpoint();
        this.temporaryLink.setAttribute("marker-end", "url(#marker-circle)");
        this.temporaryLink.setAttribute('x2', "" + end.x);
        this.temporaryLink.setAttribute('y2', "" + end.y);
    }

    public _notifyInputConnectorMouseleave(c: FlowchartInputConnector, e: MouseEvent) {
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

    private fbd2json(): string {
        let operators: OperatorData[] = [];
        let links: LinkData[] = [];
        for (const op of this.operators.values()) {
            operators.push({ globalTypeIndex: op.TypeInfo.GlobalTypeIndex, caption: op.Caption, index: op.GlobalOperatorIndex, posX: op.Xpos, posY: op.Ypos, configurationData: op.Config_Copy });
        }
        for (const link of this.links.values()) {
            links.push({
                color: "blue",
                fromOperatorIndex: link.From.Parent.GlobalOperatorIndex,
                fromOutput: link.From.LocalConnectorIndex,
                toOperatorIndex: link.To.Parent.GlobalOperatorIndex,
                toInput: link.To.LocalConnectorIndex,
            });
        }
        let data: FlowchartData = { operators: operators, links: links };
        return JSON.stringify(data);
    }

    private saveJSONToLocalFile() {
        let text = this.fbd2json();
        let filename = "functionBlockDiagram.json";
        var element = document.createElement('a');
        element.setAttribute('href', 'data:text/plain;charset=utf-8,' + encodeURIComponent(text));
        element.setAttribute('download', filename);
        element.style.display = 'none';
        document.body.appendChild(element);
        element.click();
        document.body.removeChild(element);
    }

    private saveBinToLocalFile() {
        let compilerInstance = new FlowchartCompiler(this.operators);
        let binFile = compilerInstance.Compile();
        let blob = new Blob([new Uint8Array(binFile.buf, 0, binFile.buf.byteLength)], { type: "octet/stream" });
        let url = window.URL.createObjectURL(blob);
        let filename = "functionBlockDiagram.bin";
        var element = document.createElement('a');
        element.style.display = 'none';
        element.href = url;
        element.download = filename;
        document.body.appendChild(element);
        element.click();
        document.body.removeChild(element);
    }

    private openFromLocalFile(files: FileList | null) {
        if (files == null || files.length != 1) return;
        const reader = new FileReader();
        reader.onloadend = (e) => {
            let s: string = <string>e.target!.result;
            let data = <FlowchartData>JSON.parse(s);
            this.setData(data);
        }
        reader.readAsText(files[0]);
    }

    private onResponseFbdRun(m:ResponseFbdRun){
        this.appManagement.ShowSnackbar(Severity.SUCCESS,`File now runs on Lab@Home`);
    }

    private postFbdFileToLabathome(path:string, onSuccessAction?:(path:string)=>void, onFailAction?:(path:string)=>void){
        var compilerInstance = new FlowchartCompiler(this.operators);
        var guidAndBufAndMap: HashAndBufAndMaps = compilerInstance.Compile();
        var view1 = new DataView(new ArrayBuffer(4));
        view1.setUint32(0, guidAndBufAndMap.buf.byteLength, true); //ESP32 is little-endian: true für little-endian, false für big-endian
        var stringBuffer = new TextEncoder().encode(this.fbd2json()).buffer;
        var combinedBuffer = new Uint8Array(view1.byteLength + guidAndBufAndMap.buf.byteLength + stringBuffer.byteLength);
        combinedBuffer.set(new Uint8Array(view1.buffer), 0);
        combinedBuffer.set(new Uint8Array(guidAndBufAndMap.buf), 4);
        combinedBuffer.set(new Uint8Array(stringBuffer), 4 + guidAndBufAndMap.buf.byteLength);

        let xhr_json = new XMLHttpRequest;
        xhr_json.open("POST",this.options.httpServerBasePath+path, true);
        xhr_json.onloadend = (e) => {
            if(xhr_json.status!=200){
                this.appManagement.ShowDialog(new OkDialog(Severity.ERROR, `HTTP Error ${xhr_json.status}`));
                if(onFailAction)onFailAction(path)
                return;
            }
            this.appManagement.ShowSnackbar(Severity.SUCCESS, `Successfully saved`);
            if(onSuccessAction)onSuccessAction(path)
        }
        xhr_json.onerror = (e) => { 
            this.appManagement.ShowDialog(new OkDialog(Severity.ERROR, `Generic Error`))
            if(onFailAction)onFailAction(path)    
        }
        xhr_json.send(combinedBuffer);
    }

    private getFbdFileFromLabathome(path:string){
        let xhr = new XMLHttpRequest;
        xhr.open("GET", this.options.httpServerBasePath+path, true);
        xhr.responseType="arraybuffer"
        xhr.onload = (e) => {
            let s = xhr.response as ArrayBuffer;
            var dv = new DataView(s);
            var sizeOfBinary= dv.getUint32(0, true);
            //var the_binary = s.slice(4, 4+sizeOfBinary);
            var the_json = new TextDecoder().decode(s.slice(4+sizeOfBinary));
            this.setData(<FlowchartData>JSON.parse(the_json));
        }
        xhr.send();
    }

    private deleteFdbFileFromLabathome(path:string){
        let xhr = new XMLHttpRequest;
        xhr.open("DELETE", this.options.httpServerBasePath+path, true); //GET with the filename selected in the dialog
        xhr.onloadend = (e) => {
            this.appManagement.ShowSnackbar(Severity.SUCCESS, `File ${path} deleted successfully`);
        }
        xhr.send();
    }

    private getFbdFileListFromLabathome(path_with_slash_at_the_end:string){
        let xhr = new XMLHttpRequest;
        xhr.open("GET", this.options.httpServerBasePath+path_with_slash_at_the_end, true);
        xhr.onload = (e) => {
            let s = xhr.responseText;
            let data = <{files:string[], dirs:string[]}>JSON.parse(s);
            this.appManagement.ShowDialog(new FilelistDialog(data.files,
                (ok:boolean, filename:string)=>{
                    if(!ok) return;
                    this.getFbdFileFromLabathome(path_with_slash_at_the_end+filename);
                },
                (ok:boolean, filename:string)=>{
                    if(!ok) return;
                    this.deleteFdbFileFromLabathome(path_with_slash_at_the_end+filename);
                }
            ));
        }
        xhr.send();
    }

    private enterFilenameAndPostFbd(){
        this.appManagement.ShowDialog(new FilenameDialog("Enter filename (without Extension", (ok:boolean, filename:string)=>{
            if(!ok) return
            this.postFbdFileToLabathome(FBDSTORE_BASE_DIRECTORY+filename+".fbd")
        }));

    }
    


    private buildMenu(subcontainer: HTMLDivElement) {
        let fileInput = <HTMLInputElement>Html(subcontainer, "input", ["type", "file", "id", "fileInput", "accept", ".json"]);
        fileInput.style.display = "none";
        fileInput.onchange = (e) => {
            this.openFromLocalFile(fileInput.files);
        }
        var mm:MenuManager=new MenuManager(
            [
                new Menu("File", [
                    new MenuItem("📂 Open (Local)", ()=>fileInput.click()),
                    new MenuItem("📂 Open (labathome)", ()=>this.getFbdFileListFromLabathome(FBDSTORE_BASE_DIRECTORY)),
                    new MenuItem("📂 Open Default (labathome)", ()=>this.getFbdFileFromLabathome(DEFAULT_FBD_FILEPATH)),
                    new MenuItem("💾 Save (Local)", ()=>this.saveJSONToLocalFile()),
                    new MenuItem("💾 Save (labathome)", ()=>this.enterFilenameAndPostFbd()),
                    new MenuItem("💾 Save Bin (Local)", ()=>this.saveBinToLocalFile()),
                ]),
                new Menu("Debug",[
                    new MenuItem("☭ Start Debug", ()=>this.postFbdFileToLabathome(TEMP_FBD_FILEPATH, 
                        (p:string)=>{
                            let b = new flatbuffers.Builder(1024);
                            b.finish(RequestFbdRun.createRequestFbdRun(b));
                            this.appManagement.WrapAndSend(Namespace.Value, b, 3000);
                            this.mode=FlowchartMode.DEBUG;
                        },
                        (p:string)=>{
                           console.error(`As file "${p}" could no be saved on labathome, the RequestFbdRun will not be sent to labathome`)
                        }
                    )),
                    new MenuItem("× Stop Debug", ()=>this.mode=FlowchartMode.EDIT), 
                    new MenuItem("👣 Set as Startup-App", ()=>this.postFbdFileToLabathome(DEFAULT_FBD_FILEPATH)), 
                ]),
                new Menu("Simulation",[
                    new MenuItem("➤ Start Simulation", ()=>{
                        let compilerInstance = new FlowchartCompiler(this.operators);
                        this.simulationManager = new SimulationManager(compilerInstance.CompileForSimulation());
                        this.simulationManager.Start(false);
                        this.mode=FlowchartMode.SIMULATE;
                    }),
                    new MenuItem("× Stop Simulation",()=>{
                        this.simulationManager?.Stop();
                        this.mode=FlowchartMode.EDIT;
                    }) 
                ])
            ]
        );
        mm.Render(subcontainer)
    }

    public RenderUi(subcontainer: HTMLDivElement) {
        if (!subcontainer) throw new Error("container is null");
        //let subcontainer = <HTMLDivElement>Html(container, "div", [], ["develop-ui"]);
        

        this.buildMenu(subcontainer);

        let workspace = <HTMLDivElement>Html(subcontainer, "div", ["tabindex", "0"], ["develop-workspace"]);//tabindex, damit keypress-Events abgefangen werden können
        this.propertyGridHtmlDiv = <HTMLDivElement>Html(subcontainer, "div", [], ["develop-properties"]);

        this.flowchartContainerSvgSvg = <SVGSVGElement>Svg(workspace, "svg", ["width", "100%", "height", "100%"], ["flowchart-container"]);

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
        operatorLibActivator.onmouseenter = (e) => {
            this.operatorLibDiv.style.display = "inline";
        }
        this.operatorLibDiv.onmouseleave = (e) => {
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

        this.getFbdFileFromLabathome(DEFAULT_FBD_FILEPATH);
        this.recreateFlowchartFromData();
    }

    constructor(private appManagement: IAppManagement, private flowchartData: FlowchartData = null, private flowchartCallbacks: FlowchartCallback, private options: FlowchartOptions) {
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
        let indexInData2operator = new Map<number, FlowchartOperator>();

        for (const d of this.flowchartData.operators) {
            let o = this.createOperatorInternal(d.globalTypeIndex, d.caption, d.configurationData);
            o.MoveTo(d.posX, d.posY);
            this.operators.set(o.GlobalOperatorIndex, o);
            indexInData2operator.set(d.index, o);
        }
        for (const d of this.flowchartData.links) {
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
        this.markerArrow.style.fill = color;
        this.markerCircle.style.fill = color;
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
            Html(this.propertyGridHtmlDiv, "button", [], ["develop-propertygrid-button"], `Save`).onclick = (e) => {
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
