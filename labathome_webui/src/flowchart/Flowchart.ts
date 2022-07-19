import { ConnectorType, FlowchartInputConnector, FlowchartOutputConnector } from "./FlowchartConnector";
import { FlowchartCompiler, HashAndBufAndMaps } from "./FlowchartCompiler";
import { FlowchartLink } from "./FlowchartLink";
import { FlowchartOperator, TypeInfo } from "./FlowchartOperator";
import * as operatorimpl from "./FlowchartOperatorImpl";
import { Utils, $, KeyValueTuple } from "../utils";
import { AppManagement } from "../AppManagement";
import { SerializeContext } from "./SerializeContext";
import { SimulationManager } from "./SimulationManager";

const URL_PREFIX="";

//const URL_PREFIX="http://labathome-ed5564";

export class FlowchartOptions {
    canUserEditLinks: boolean = true;
    canUserMoveOperators: boolean = true;
    data?: FlowchartData = undefined;
    distanceFromArrow: number = 3;
    defaultOperatorClass: string = 'flowchart-default-operator';
    defaultLinkColor: string = '#3366ff';
    defaultSelectedLinkColor: string = 'black';
    linkWidth: number = 10;
    grid: number = 10;
    multipleLinksOnOutput: boolean = true;
    multipleLinksOnInput: boolean = false;
    linkVerticalDecal: number = 0;
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

export interface FlowchartData {
    operators: OperatorData[];
    links: LinkData[];
}

export interface OperatorData {
    globalTypeIndex: number;
    caption: string;
    index: number;
    posX: number;
    posY: number;
    configurationData: KeyValueTuple[] | null;
}



export interface LinkData {
    color: string;
    fromOperatorIndex: number;
    fromOutput: number;
    toOperatorIndex: number;
    toInput: number;
}

export class Flowchart {
    
    private operatorRegistry:operatorimpl.OperatorRegistry;
    private simulationManager?:SimulationManager|null;
    private operators = new Map<number, FlowchartOperator>();
    private links = new Map<number, FlowchartLink>();
    public static readonly DATATYPE2COLOR = new Map([[ConnectorType.BOOLEAN, "RED"], [ConnectorType.COLOR, "GREEN"], [ConnectorType.FLOAT, "BLUE"], [ConnectorType.INTEGER, "YELLOW"], [ConnectorType.COLOR, "PURPLE"]]);
    //Muss beim Löschen+Erzeugen von Operatoren+Links und bei Speichern von Properties zurückgesetzt werden
    private currentDebugInfo:HashAndBufAndMaps|null=null;
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

    public triggerDebug() {
        if(this.currentDebugInfo==null) return;

        let xhr = new XMLHttpRequest;
        xhr.onerror = (e) => { console.error("Fehler beim XMLHttpRequest!"); };
        xhr.open("GET", URL_PREFIX+"/fbd", true);
        xhr.responseType = "arraybuffer";
        xhr.onload = (e) => {
            if(this.currentDebugInfo==null) return;
            
            let arrayBuffer = xhr.response; // Note: not oReq.responseText
            if (!arrayBuffer || arrayBuffer.byteLength <=16) {
                console.error("! arrayBuffer || arrayBuffer.byteLength<16");
                this.currentDebugInfo=null;
                return;
            }
            let ctx = new SerializeContext(arrayBuffer);
            let hash = ctx.readU32();
            if(hash!=this.currentDebugInfo.hash){
                console.error("hash!=this.currentDebugInfo.hash");
                this.currentDebugInfo=null;
                return;
            }
            let binaryCount = ctx.readU32();
            for(let adressOffset=0;adressOffset<binaryCount;adressOffset++)
            {
                let value = ctx.readU32();
                if(adressOffset<2) continue;
                let connectorType=ConnectorType.BOOLEAN
                let map = this.currentDebugInfo.typeIndex2adressOffset2ListOfLinks.get(connectorType)!;
                let linksToChange = map.get(adressOffset);
                if(linksToChange===undefined){
                    console.error(`linksToColorize===undefined for connectorType ${connectorType} addressOffset ${adressOffset} and value ${value}`);
                    continue;
                }
                linksToChange.forEach((e)=>{
                    e.SetColor(value==1?"red":"grey");
                    e.SetCaption(""+value);
                });
            }

            let integerCount = ctx.readU32();
            for(let adressOffset=0;adressOffset<integerCount;adressOffset++)
            {
                let value = ctx.readS32();
                if(adressOffset<2) continue;
                let connectorType=ConnectorType.INTEGER
                let map = this.currentDebugInfo.typeIndex2adressOffset2ListOfLinks.get(connectorType)!;
                let linksToChange = map.get(adressOffset);
                if(linksToChange===undefined){
                    console.error(`linksToColorize===undefined for connectorType ${connectorType} addressOffset ${adressOffset} and value ${value}`);
                    continue;
                }
                linksToChange.forEach((e)=>{
                    e.SetCaption(""+value);
                });
            }

            let floatsCount = ctx.readU32();
            for(let adressOffset=0;adressOffset<floatsCount;adressOffset++)
            {
                let value = ctx.readF32();
                if(adressOffset<2) continue;
                let connectorType=ConnectorType.FLOAT
                let map = this.currentDebugInfo.typeIndex2adressOffset2ListOfLinks.get(connectorType)!;
                let linksToChange = map.get(adressOffset);
                if(linksToChange===undefined){
                    console.error(`linksToColorize===undefined for connectorType ${connectorType} addressOffset ${adressOffset} and value ${value}`);
                    continue;
                }
  
                linksToChange.forEach((e)=>{
                    e.SetCaption(value.toFixed(2));
                });
            }

            let colorsCount = ctx.readU32();
            for(let adressOffset=0;adressOffset<colorsCount;adressOffset++)
            {
                let value = ctx.readU32();
                if(adressOffset<2) continue;
                let connectorType=ConnectorType.COLOR
                let map = this.currentDebugInfo.typeIndex2adressOffset2ListOfLinks.get(connectorType)!;
                let linksToChange = map.get(adressOffset);
                if(linksToChange===undefined){
                    console.error(`linksToColorize===undefined for connectorType ${connectorType} addressOffset ${adressOffset} and value ${value}`);
                    continue;
                }
                linksToChange.forEach((e)=>{
                    e.SetCaption(""+value);
                    e.SetColor($.ColorNumColor2ColorDomString(value));
                });
            }
        }
        xhr.send();
    }

    public _notifyGlobalMousemoveWithLink(e: MouseEvent) {
        if (this.lastOutputConnectorClicked != null && !this.temporaryLinkSnapped) {
            let end = Utils.EventCoordinatesInSVG(e, this.flowchartContainerSvgSvg, this.positionRatio);
            this.temporaryLink.setAttribute('x2', "" + end.x);
            this.temporaryLink.setAttribute('y2', "" + end.y);
        }
    }

    public _notifyGlobalMouseupWithLink(e: MouseEvent) {
        this.unsetTemporaryLink();
    }

    public _notifyOutputConnectorMousedown(c: FlowchartOutputConnector, e: MouseEvent) {
        this.temporaryLinkSnapped = false;
        let start = c.GetLinkpoint();
        let end = Utils.EventCoordinatesInSVG(e, this.flowchartContainerSvgSvg, this.positionRatio);
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
        if (this.lastOutputConnectorClicked == null) return;
        if (!this.options.multipleLinksOnInput && c.LinksLength > 0) return;
        if (this.lastOutputConnectorClicked.Type == c.Type) {
            this.createLink(null, this.lastOutputConnectorClicked, c);
        }
        this.unsetTemporaryLink();

    }

    public _notifyOperatorClicked(o: FlowchartOperator, e: MouseEvent) {
        this.SelectOperator(o);
    }

    public _notifyLinkClicked(link: FlowchartLink, e: MouseEvent) {
        this.selectLink(link);
    }

    public _notifyInputConnectorMouseenter(c: FlowchartInputConnector, e: MouseEvent) {
        if (this.lastOutputConnectorClicked == null || this.lastOutputConnectorClicked.Type != c.Type) return;
        if (!this.options.multipleLinksOnInput && c.LinksLength > 0) return;

        this.temporaryLinkSnapped = true;
        let end = c.GetLinkpoint();
        this.temporaryLink.setAttribute("marker-end", "url(#marker-circle)");
        this.temporaryLink.setAttribute('x2', "" + end.x);
        this.temporaryLink.setAttribute('y2', "" + end.y);
    }

    public _notifyInputConnectorMouseleave(c: FlowchartInputConnector, e: MouseEvent) {
        this.temporaryLinkSnapped = false;
        this.temporaryLink.setAttribute("marker-end", "url(#marker-arrow)");
    }

    public unselectLink() {
        if (this.selectedLink != null) {
            if (this.options.onLinkUnselect && !this.options.onLinkUnselect(this.selectedLink)) {
                return;
            }
            this.selectedLink.UnsetColor();
            this.selectedLink = null;
        }
    }

    public selectLink(link: FlowchartLink) {
        this.unselectLink();
        if (this.options.onLinkSelect && !this.options.onLinkSelect(link)) {
            return;
        }
        this.unselectOperator();
        this.selectedLink = link;
        link.SetColor(this.options.defaultSelectedLinkColor);
    }
 

    private deleteSelectedThing(): void {
        if (this.selectedOperator) {
            this.DeleteOperator(this.selectedOperator.GlobalOperatorIndex);
        }
        else if (this.selectedLink) {
            this.DeleteLink(this.selectedLink.GlobalLinkIndex);
        }
    }

    private fbd2json():string{
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
        
        let text = this.fbd2json();
        let compilerInstance = new FlowchartCompiler(this.operators);
        let binFile =compilerInstance.Compile();  
        let blob = new Blob([new Uint8Array(binFile.buf, 0, binFile.buf.byteLength)], {type: "octet/stream"});
        let url = window.URL.createObjectURL(blob);
        let filename = "functionBlockDiagram.bin";
        var element = document.createElement('a');
        element.style.display = 'none';
        element.href=url;
        element.download=filename;
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

    private put2fbd(buf:ArrayBuffer)
    {
        let xhr = new XMLHttpRequest;
        xhr.open("PUT", URL_PREFIX+"/fbd", true);
        xhr.onloadend = (e) => {
            if(xhr.status!=200){
                this.appManagement.DialogController().showOKDialog(16, `HTTP Error ${xhr.status}`, null);
                return;
            }
            this.appManagement.DialogController().showOKDialog(16, `Successfully saved`, null);
        }
        xhr.onerror = (e) => { 
            this.appManagement.DialogController().showOKDialog(16, `Generic Error`, null);
        }
        xhr.send(buf);
    }

    private saveJSONToLabathomeFile(){
        
        this.appManagement.DialogController().showEnterFilenameDialog(10, "Enter filename (without Extension", (filename:string)=>{
            let xhr_json = new XMLHttpRequest;
            xhr_json.open("POST", URL_PREFIX+"/fbdstorejson/"+filename, true);
            xhr_json.onloadend = (e) => {
                if(xhr_json.status!=200){
                    this.appManagement.DialogController().showOKDialog(16, `HTTP Error ${xhr_json.status}`, null);
                    return;
                }
                this.appManagement.DialogController().showOKDialog(16, `Successfully saved`, null);
            }
            xhr_json.onerror = (e) => { this.appManagement.DialogController().showOKDialog(16, `Generic Error`, null);}
            xhr_json.send(this.fbd2json());
        });

    }

    private saveJSONandBINToLabathomeDefaultFile(buf:ArrayBuffer)
    {
        let xhr_bin = new XMLHttpRequest();
        xhr_bin.open("POST", URL_PREFIX+"/fbddefaultbin", true);
        xhr_bin.onloadend = (e) => {
            if(xhr_bin.status!=200){
                this.appManagement.DialogController().showOKDialog(16, `HTTP Error ${xhr_bin.status}`, null);
                return;
            }
            let xhr_json  = new XMLHttpRequest();
            xhr_json.open("POST", URL_PREFIX+"/fbddefaultjson", true);
            xhr_json.onloadend =(e)=>{
                if(xhr_json.status!=200){
                    this.appManagement.DialogController().showOKDialog(16, `HTTP Error ${xhr_json.status}`, null);
                    return;
                }
                this.appManagement.DialogController().showOKDialog(16, `Successfully set a new default FBD`, null);
            }
            xhr_json.onerror = (e) => { this.appManagement.DialogController().showOKDialog(16, `Generic Error`, null);}
            xhr_json.send(this.fbd2json())
        }
        xhr_bin.onerror = (e) => { this.appManagement.DialogController().showOKDialog(16, `Generic Error`, null);}
        xhr_bin.send(buf);
    }

    private openJSONFromLabathome()
    {
        let filename:string = "";
        let xhr = new XMLHttpRequest;
        xhr.open("GET", URL_PREFIX+"/fbdstorejson/", true);//GET without filename, but with "/" at the end!!!
        xhr.onload = (e) => {
            let s = xhr.responseText;
            let data = <string[]>JSON.parse(s);
            this.appManagement.DialogController().showFilelist(1000, data, 
                (filename:string)=>{
                    let xhr = new XMLHttpRequest;
                    xhr.open("GET", URL_PREFIX+"/fbdstorejson/"+filename, true); //GET with the filename selected in the dialog
                    xhr.onload = (e) => {
                        let s = xhr.responseText;
                        let data = <FlowchartData>JSON.parse(s);
                        this.setData(data);
                    }
                    xhr.send();
                },
                (filename:string)=>{
                    let xhr = new XMLHttpRequest;
                    xhr.open("DELETE", URL_PREFIX+"/fbdstorejson/"+filename, true); //GET with the filename selected in the dialog
                    xhr.onloadend = (e) => {
                        this.appManagement.DialogController().showOKDialog(1, `File ${filename} deleted successfully`, null);
                    }
                    xhr.send();
                }
            );
        }
        xhr.send();
    }


    private openDefaultJSONFromLabathome()
    {
        let xhr = new XMLHttpRequest;
        xhr.open("GET", URL_PREFIX+"/fbddefaultjson", true);
        xhr.onload = (e) => {
            let s = xhr.responseText;
            let data = <FlowchartData>JSON.parse(s);
            this.setData(data);
        }
        xhr.send();
    }

    

    private buildMenu(subcontainer: HTMLDivElement) {
        let fileInput = <HTMLInputElement>$.Html(subcontainer, "input", ["type", "file", "id", "fileInput", "accept", ".json"]);
        fileInput.style.display = "none";
        fileInput.onchange = (e) => {
            this.openFromLocalFile(fileInput.files);
        }

        let toolbar = $.Html(subcontainer, "div", [], ["develop-toolbar"]);
        let menuFile = $.Html(toolbar, "div", [], ["dropdown"]);
        let menuFileDropBtn = <HTMLButtonElement>$.Html(menuFile, "button", [], ["dropbtn"], "File ▼");
        let menuFileDropContent = $.Html(menuFile, "div", [], ["dropdown-content"]);
        menuFileDropBtn.onclick = (e) => { menuFileDropContent.classList.toggle("show"); };
        $.Html(menuFileDropContent, "a", ["href", "#"], [], "📂 Open (Local)").onclick = (e) => {
            Array.prototype.forEach.call(document.getElementsByClassName("dropdown-content"), (elem: HTMLDivElement) => { elem.classList.remove("show"); });
            fileInput.click();
            e.preventDefault();
        }
        $.Html(menuFileDropContent, "a", ["href", "#"], [], "📂 Open (labathome)").onclick = (e) => {
            Array.prototype.forEach.call(document.getElementsByClassName("dropdown-content"), (elem: HTMLDivElement) => { elem.classList.remove("show"); });
            this.openJSONFromLabathome()
            e.preventDefault();
        }
        $.Html(menuFileDropContent, "a", ["href", "#"], [], "📂 Open Default (labathome)").onclick = (e) => {
            Array.prototype.forEach.call(document.getElementsByClassName("dropdown-content"), (elem: HTMLDivElement) => { elem.classList.remove("show"); });
            this.openDefaultJSONFromLabathome()
            e.preventDefault();
        }
        $.Html(menuFileDropContent, "a", ["href", "#"], [], "💾 Save (Local)").onclick = (e) => {
            Array.prototype.forEach.call(document.getElementsByClassName("dropdown-content"), (elem: HTMLDivElement) => { elem.classList.remove("show"); });
            this.saveJSONToLocalFile();
            e.preventDefault();
        }
        
        $.Html(menuFileDropContent, "a", ["href", "#"], [], "💾 Save (labathome)").onclick = (e) => {
            Array.prototype.forEach.call(document.getElementsByClassName("dropdown-content"), (elem: HTMLDivElement) => { elem.classList.remove("show"); });
            this.saveJSONToLabathomeFile();
            e.preventDefault();
        }
        $.Html(menuFileDropContent, "a", ["href", "#"], [], "💾 Save Bin (Local)").onclick = (e) => {
            Array.prototype.forEach.call(document.getElementsByClassName("dropdown-content"), (elem: HTMLDivElement) => { elem.classList.remove("show"); });
            this.saveBinToLocalFile();
            e.preventDefault();
        }
        //let runbutton = $.Html(toolbar, "a", ["href", "#"], ["develop-toolbar"], "Run");

        let menuDebug = $.Html(toolbar, "div", [], ["dropdown"]);
        let menuDebugDropBtn = <HTMLButtonElement>$.Html(menuDebug, "button", [], ["dropbtn"], "Debug ▼");

        let menuDebugDropContent = $.Html(menuDebug, "div", [], ["dropdown-content"]);
        menuDebugDropBtn.onclick = (e) => {
            menuDebugDropContent.classList.toggle("show");
        };
        $.Html(menuDebugDropContent, "a", ["href", "#"], [], "☭ Run Now").onclick = (e) => {
            Array.prototype.forEach.call(document.getElementsByClassName("dropdown-content"), (elem: HTMLDivElement) => { elem.classList.remove("show"); });
            let compilerInstance = new FlowchartCompiler(this.operators);
            let guidAndBufAndMap: HashAndBufAndMaps=compilerInstance.Compile();  
            this.currentDebugInfo=guidAndBufAndMap;
            this.put2fbd(guidAndBufAndMap.buf);
            e.preventDefault();
        }
        $.Html(menuDebugDropContent, "a", ["href", "#"], [], "👣 Set as Startup-App").onclick = (e) => {
            Array.prototype.forEach.call(document.getElementsByClassName("dropdown-content"), (elem: HTMLDivElement) => { elem.classList.remove("show"); });
            let compilerInstance = new FlowchartCompiler(this.operators);
            let guidAndBufAndMap: HashAndBufAndMaps=compilerInstance.Compile();  
            this.saveJSONandBINToLabathomeDefaultFile(guidAndBufAndMap.buf);
            e.preventDefault();
        }
        let menuSimulation = $.Html(toolbar, "div", [], ["dropdown"]);
        let menuSimulationDropBtn = <HTMLButtonElement>$.Html(menuSimulation, "button", [], ["dropbtn"], "Simulation ▼");

        let menuSimulationDropContent = $.Html(menuSimulation, "div", [], ["dropdown-content"]);
        menuSimulationDropBtn.onclick = (e) => {
            menuSimulationDropContent.classList.toggle("show");
        };
        $.Html(menuSimulationDropContent, "a", ["href", "#"], [], "➤ Start Simulation").onclick = (e) => {
            Array.prototype.forEach.call(document.getElementsByClassName("dropdown-content"), (elem: HTMLDivElement) => { elem.classList.remove("show"); });
            let compilerInstance = new FlowchartCompiler(this.operators);
            this.simulationManager=new SimulationManager(compilerInstance.CompileForSimulation());
            this.simulationManager.Start(false);
            e.preventDefault();
        }
        $.Html(menuSimulationDropContent, "a", ["href", "#"], [], "× Stop Simulation").onclick = (e) => {
            Array.prototype.forEach.call(document.getElementsByClassName("dropdown-content"), (elem: HTMLDivElement) => { elem.classList.remove("show"); });
            this.simulationManager?.Stop();
            e.preventDefault();
        }
        //let menuDebugLink2 = $.Html(menuDebugDropContent, "a", ["href", "#"], [], "◉ Stop");
        //let menuDebugLink3 = $.Html(menuDebugDropContent, "a", ["href", "#"], [], "◯ Erase");
    }

    constructor(private appManagement:AppManagement, private container: HTMLDivElement, private options: FlowchartOptions) {
        if (!this.container) throw new Error("container is null");
        this.operatorRegistry=operatorimpl.OperatorRegistry.Build();
        let subcontainer = <HTMLDivElement>$.Html(this.container, "div", [], ["develop-ui"]);
        subcontainer.onclick = (e) => {
            if ((<HTMLElement>e.target).classList.contains("dropbtn")) return;
            Array.prototype.forEach.call(document.getElementsByClassName("dropdown-content"), (elem: HTMLDivElement) => { elem.classList.remove("show"); });
        }

        this.buildMenu(subcontainer);


        let workspace = <HTMLDivElement>$.Html(subcontainer, "div", ["tabindex", "0"], ["develop-workspace"]);//tabindex, damit keypress-Events abgefangen werden können
        this.propertyGridHtmlDiv = <HTMLDivElement>$.Html(subcontainer, "div", [], ["develop-properties"]);



        this.flowchartContainerSvgSvg = <SVGSVGElement>$.Svg(workspace, "svg", ["width", "100%", "height", "100%"], ["flowchart-container"]);


        this.linksLayer = <SVGGElement>$.Svg(this.flowchartContainerSvgSvg, "g", [], ["flowchart-links-layer"]);
        this.operatorsLayer = <SVGGElement>$.Svg(this.flowchartContainerSvgSvg, "g", [], ["flowchart-operators-layer", "unselectable"]);
        this.tempLayer = <SVGSVGElement>$.Svg(this.flowchartContainerSvgSvg, "g", [], ["flowchart-temporary-link-layer"]);
        this.tempLayer.style.visibility = "hidden";//visible
        let defs = $.Svg(this.tempLayer, "defs", []);
        let markerArrow = $.Svg(defs, "marker", ["id", "marker-arrow", "markerWidth", "4", "markerHeight", "4", "refX", "1", "refY", "2", "orient", "0"]);
        this.markerArrow = <SVGPathElement>$.Svg(markerArrow, "path", ["d", "M0,0 L0,4 L2,2 z", "fill", "red", "stroke", "black", "stroke-width", "0.5"]);
        let markerCircle = $.Svg(defs, "marker", ["id", "marker-circle", "markerWidth", "4", "markerHeight", "4", "refX", "2", "refY", "2", "orient", "0"]);
        this.markerCircle = <SVGCircleElement>$.Svg(markerCircle, "circle", ["cx", "2", "cy", "2", "r", "2", "fill", "red", "stroke-width", "1px", "stroke", "black"]);
        this.temporaryLink = <SVGLineElement>$.Svg(this.tempLayer, "line", ["x1", "0", "y1", "0", "x2", "0", "y2", "0", "stroke-dasharray", "6,6", "stroke-width", "4", "stroke", "black", "fill", "none", "marker-end", "url(#marker-arrow)"]);

        let operatorLibActivator = <SVGRectElement>$.Svg(this.flowchartContainerSvgSvg, "rect", ["width", "40", "height", "100%", "fill", "white", "fill-opacity", "0"]);

        this.operatorLibDiv = <HTMLDivElement>$.Html(workspace, "div", [], ["flowchart-operatorlibdiv", "unselectable"]);
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

        this.operatorRegistry.populateOperatorLib(this.operatorLibDiv,(e:MouseEvent, ti:TypeInfo)=>{
            let caption = ti.OperatorName;
            let o = this.createOperatorInternal(ti.GlobalTypeIndex, caption, null);
            let coords = Utils.EventCoordinatesInSVG(e, this.Element);
            o.MoveTo(coords.x - 10, coords.y - 10);
            o.RegisterDragging(e);
            this.operators.set(o.GlobalOperatorIndex, o);
        } );
    }


    public onFirstStart() {
        if (typeof this.options.data !== undefined && this.options.data != null) {
            this.setData(this.options.data);
        }
    }


    private createOperatorInternal(globalTypeIndex: number, caption: string, configurationData: KeyValueTuple[] | null): FlowchartOperator {
        
        if(!this.operatorRegistry.IsIndexKnown(globalTypeIndex))
        {
            throw new Error(`Unknown globalTypeIndex ${globalTypeIndex}`);
        }
        if (this.options.onOperatorCreate && !this.options.onOperatorCreate(caption, null, false)) {
            throw new Error(`Creation of operator of globalTypeIndex ${globalTypeIndex} prevented by onOperatorCreate plugin`);
        }
        let op = this.operatorRegistry.CreateByIndex(globalTypeIndex, this, caption, configurationData)!;
       
        this.currentDebugInfo=null;
        return op;
    }

    public setData(data: FlowchartData) {

        this.links.forEach((e) => e.RemoveFromDOM());
        this.links.clear();
        this.operators.forEach((e) => e.RemoveFromDOM());
        this.operators.clear();
        let indexInData2operator = new Map<number, FlowchartOperator>();

        for (const d of data.operators) {
            let o = this.createOperatorInternal(d.globalTypeIndex, d.caption, d.configurationData);
            o.MoveTo(d.posX, d.posY);
            this.operators.set(o.GlobalOperatorIndex, o);
            indexInData2operator.set(d.index, o);
        }
        for (const d of data.links) {
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
        this.currentDebugInfo=null;
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
        this.currentDebugInfo=null;
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
        if (this.options.onLinkCreate && !this.options.onLinkCreate(from.Caption, data)) return null;
        if (!this.options.multipleLinksOnOutput && from.LinksLength > 0) return null;
        if (!this.options.multipleLinksOnInput && to.LinksLength > 0) return null;
        this.currentDebugInfo=null;
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
        if (this.options.onOperatorUnselect && !this.options.onOperatorUnselect()) return;
        this.propertyGridHtmlDiv.innerText = ""; //clear
        if (this.selectedOperator == null) return;
        this.selectedOperator.ShowAsSelected(false);
        this.selectedOperator = null;
    }

    public SelectOperator(operator: FlowchartOperator): void {
        if (this.options.onOperatorSelect && !this.options.onOperatorSelect(operator.Caption)) return;
        this.unselectLink();
        if (this.selectedOperator != null) this.selectedOperator.ShowAsSelected(false);
        operator.ShowAsSelected(true);
        this.selectedOperator = operator;
        this.propertyGridHtmlDiv.innerText = ""; //clear
        $.Html(this.propertyGridHtmlDiv, "p", [], ["develop-propertygrid-head"], `Properties for ${this.selectedOperator.Caption}`);
        let table = <HTMLTableElement>$.Html(this.propertyGridHtmlDiv, "table", [], ["develop-propertygrid-table"]);
        let thead = <HTMLTableSectionElement>$.Html(table, "thead", [],[]);
        let tr_head = $.Html(thead, "tr", [], ["develop-propertygrid-tr"]);
        $.Html(tr_head, "th", [], ["develop-propertygrid-th"], "Key");
        $.Html(tr_head, "th", [], ["develop-propertygrid-th"], "Value");
        let tbody= <HTMLTableSectionElement>$.Html(table, "tbody", [],[]);
        if (this.selectedOperator!.PopulateProperyGrid(tbody)) {
            $.Html(this.propertyGridHtmlDiv, "button", [], ["develop-propertygrid-button"], `Save`).onclick=(e)=>{
                this.currentDebugInfo=null;
                operator.SavePropertyGrid(tbody);
            };
        }
        else {
            this.propertyGridHtmlDiv.innerText = ""; //clear
            $.Html(this.propertyGridHtmlDiv, "p", [], ["develop-propertygrid-head"], `No Properties for ${this.selectedOperator.Caption}`);
        }
    }

    // Found here : http://stackoverflow.com/questions/5560248/programmatically-lighten-or-darken-a-hex-color-or-rgb-and-blend-colors
    public static _shadeColor(color: string, percent: number) {
        var f = parseInt(color.slice(1), 16), t = percent < 0 ? 0 : 255, p = percent < 0 ? percent * -1 : percent, R = f >> 16, G = f >> 8 & 0x00FF, B = f & 0x0000FF;
        return "#" + (0x1000000 + (Math.round((t - R) * p) + R) * 0x10000 + (Math.round((t - G) * p) + G) * 0x100 + (Math.round((t - B) * p) + B)).toString(16).slice(1);
    }
}
