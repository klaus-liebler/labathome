import { ConnectorType, FlowchartInputConnector, FlowchartOutputConnector } from "./FlowchartConnector";
import { FlowchartSerializer } from "./FlowchartSerializer";
import { FlowchartLink } from "./FlowchartLink";
import { FlowchartOperator, PositionType, TypeInfo } from "./FlowchartOperator";
import * as operatorimpl from "./FlowchartOperatorImpl";
import { NodeWrapper, TopologicalSortDFS } from "./TopologicalSorfDFS";
import { Utils } from "./Utils";
/*
In einem Ressources-Toolbar befinden sich alle Ressourcen (Input und Output), die ein Board anbietet. Diese können genau einmal auf das Board gezogen werden
Problem: OneWire-Ressourcen!: 
Operator-Klassen haben ein Positionstyp: Default, Input, Output
Operator-Klassen haben einen Singleton-Typ, nämlich Default, Singleton. Bei Singletons darf nur eine Instanz der KLasse erzeugt werden
Operator-Instanzen können per Propery-Grid konfiguriert werden. Sie stellen eine Methode PopulateProperyGrid(HMTLDivElement) zur Verfügung, das ein editerbares HTML-Grid ins DIV hineinzeichnen. Außerhalb wurde dieses div zuvor geleert und im Anschluss wird von außerhalb ein Save-Button gerendert.
  Ein null-Rückgabewert bedeutet, dass kein PropertyGrid benötigt wird.
  Sie stellen weiterhin eine Methode SaveProperyGrid(HMTLDivElement) zur Verfügung, in der sie die Inhalte wieder einlesen und intern wie auch immer speichern
  Sie stellen weiterhin eine Methode GetPropertyGridDataAsJSONString() zur Verfügung. Diese gibt die Daten als JSON-String zurück

*/

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
    onOperatorSelect?: (operatorId: string)=>boolean ;
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
        typeName: string;
        caption: string;
        index:number;
        posX: number;
        posY: number;
        configurationData:KeyValueTuple[]|null;
    }

    export interface KeyValueTuple
    {
        key:string;
        value:any;
    }

    export interface LinkData {
        color: string;
        fromOperatorIndex: number;
        fromOutput: number;
        toOperatorIndex: number;
        toInput: number;
    }

    //Connector besteht aus "wrapper", dem Label, dem großen dreieck und einem kleinen Dreieck

    export class Flowchart {
        private operators = new Map<number, FlowchartOperator>();
        private links = new Map<number, FlowchartLink>();
        public static readonly SVGNS = "http://www.w3.org/2000/svg";
        public static readonly XLINKNS = "http://www.w3.org/1999/xlink";
        public static readonly HTMLNS = "http://www.w3.org/1999/xhtml";
        public static readonly DATATYPE2COLOR = new Map([[ConnectorType.BOOLEAN, "RED"], [ConnectorType.COLOR, "GREEN"], [ConnectorType.FLOAT, "BLUE"], [ConnectorType.INTEGER, "YELLOW"], [ConnectorType.COLOR, "PURPLE"]]);

  
        private lastOutputConnectorClicked: FlowchartOutputConnector|null=null;
        private selectedOperator: FlowchartOperator|null=null;
        private selectedLink: FlowchartLink|null=null;
        get SelectedLink() { return this.selectedLink };
        
        private positionRatio: number = 1;
        get PositionRatio() { return this.positionRatio; }
        
        private flowchartContainerSvgSvg:SVGSVGElement
        private linksLayer: SVGGElement;
        get LinkLayer() { return this.linksLayer; }
        private operatorsLayer: SVGGElement;
        get OperatorsLayer() { return this.operatorsLayer; }
        private toolsLayer:SVGGElement;
        get ToolsLayer() { return this.toolsLayer; }
        private tempLayer:SVGGElement;
        private temporaryLink: SVGLineElement;
        private temporaryLinkSnapped=false;
        private propertyGridHtmlDiv:HTMLDivElement;

        private markerArrow:SVGPathElement;
        private markerCircle:SVGCircleElement;

        private upcounter=0;

        public _notifyGlobalMousemoveWithLink(e:MouseEvent)
        {
            if (this.lastOutputConnectorClicked != null && !this.temporaryLinkSnapped) {
                let end = Utils.EventCoordinatesInSVG(e, this.flowchartContainerSvgSvg, this.positionRatio);
                this.temporaryLink.setAttribute('x2', ""+end.x);
                this.temporaryLink.setAttribute('y2', ""+end.y);
            }
        }

        public _notifyGlobalMouseupWithLink(e:MouseEvent)
        {
            this.unsetTemporaryLink();
        }

        

        public _notifyOutputConnectorMousedown(c: FlowchartOutputConnector, e: MouseEvent) {
            this.temporaryLinkSnapped=false;
            let start = c.GetLinkpoint();
            let end = Utils.EventCoordinatesInSVG(e, this.flowchartContainerSvgSvg, this.positionRatio);
            this.temporaryLink.setAttribute('x1', ""+start.x);
            this.temporaryLink.setAttribute('y1', ""+start.y);
            this.temporaryLink.setAttribute('x2', ""+end.x);
            this.temporaryLink.setAttribute('y2', ""+end.y);
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

        public _notifyInputConnectorMouseup(c: FlowchartInputConnector, e:MouseEvent) {
            if (this.lastOutputConnectorClicked == null) return;
            if(!this.options.multipleLinksOnInput && c.LinksLength>0) return;
            if(this.lastOutputConnectorClicked.Type==c.Type)
            {
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

        public _notifyInputConnectorMouseenter(c:FlowchartInputConnector, e:MouseEvent){
            if (this.lastOutputConnectorClicked == null || this.lastOutputConnectorClicked.Type!=c.Type) return;
            if(!this.options.multipleLinksOnInput && c.LinksLength>0) return;

            this.temporaryLinkSnapped=true;
            let end= c.GetLinkpoint();
            this.temporaryLink.setAttribute("marker-end", "url(#marker-circle)");
            this.temporaryLink.setAttribute('x2', ""+end.x);
            this.temporaryLink.setAttribute('y2', ""+end.y);
        }

        public _notifyInputConnectorMouseleave(c:FlowchartInputConnector, e:MouseEvent){
            this.temporaryLinkSnapped=false;
            this.temporaryLink.setAttribute("marker-end", "url(#marker-arrow)");
        }

        public unselectLink() {
            if (this.selectedLink != null) {
                if (this.options.onLinkUnselect && !this.options.onLinkUnselect(this.selectedLink)) {
                    return;
                }
                this.selectedLink.UncolorizeLink();
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
            link.ColorizeLink(this.options.defaultSelectedLinkColor);
        }

        private compile():void{
            let index2wrappedOperator = new Map<number,NodeWrapper<FlowchartOperator>>();
            this.operators.forEach((v,k,m)=>{
                index2wrappedOperator.set(v.GlobalOperatorIndex, new NodeWrapper<FlowchartOperator>(v));
            });
            
              
            let wrappedOutputOperators:NodeWrapper<FlowchartOperator>[] =[];
            for(let i of index2wrappedOperator.values()){
                //Stelle für jede "gewrapte Node" fest, welche Operatoren von Ihr abhängig sind
                let dependents = new Set<NodeWrapper<FlowchartOperator>>();
                for (const inputkv of i.Payload.InputsKVIt) {
                    for(const linkkv of inputkv[1].LinksKVIt)
                    {
                        let dependentOperator = linkkv[1].From.Parent;
                        let dependentWrappedNode = index2wrappedOperator.get(dependentOperator.GlobalOperatorIndex);
                        if(!dependentWrappedNode) 
                            throw new Error("Implementation Error: dependentWrappedNode is undefined");
                        dependents.add(dependentWrappedNode);
                    }
                }
                dependents.forEach(e=>i.DependendNodes.push(e));
                //füge alle mit Typ "Output" einer Liste hinzu
                if(i.Payload.TypeInfo.Position==PositionType.Output) wrappedOutputOperators.push(i);
            }
            
            let algorithm = new TopologicalSortDFS<FlowchartOperator>();
            let sortedList = algorithm.sort(wrappedOutputOperators);
            for (const key in sortedList) {
                let value = sortedList[key];
                value.Payload.SetDebugInfoText("Sequenznummer "+key);
            }
            let buf = FlowchartSerializer.Serialize(sortedList.map((e)=>e.Payload));
            var xhr = new XMLHttpRequest;
            xhr.open("PUT", "/fbd", true);
            xhr.onload=(e)=>{window.alert("Erfolgreich hochgeladen");}
            xhr.onerror=(e)=>{window.alert("Fehler: "+e);}
            xhr.send(buf);
        }

        private deleteSelectedThing():void{
            if(this.selectedOperator){
                this.DeleteOperator(this.selectedOperator.GlobalOperatorIndex);
            }
            else if(this.selectedLink)
            {
                this.DeleteLink(this.selectedLink.GlobalLinkIndex);
            }
            
        }

        private saveToFile(){
            

            let operators:OperatorData[] = [];
            let links:LinkData[] = [];
            for (const op of this.operators.values()) {
                operators.push({typeName:op.TypeInfo.TypeName, caption:op.Caption, index:op.GlobalOperatorIndex, posX:op.Xpos, posY:op.Ypos, configurationData:op.Config_Copy});
            }
            for (const link of this.links.values()) {
                links.push({
                    color:"blue", 
                    fromOperatorIndex:link.From.Parent.GlobalOperatorIndex, 
                    fromOutput:link.From.LocalConnectorIndex,
                    toOperatorIndex:link.To.Parent.GlobalOperatorIndex,
                    toInput:link.To.LocalConnectorIndex,
                });
            }
            let data: FlowchartData = {operators:operators, links:links};
            let text = JSON.stringify(data);
            let filename = "functionBlockDiagram.json";
            var element = document.createElement('a');
            element.setAttribute('href', 'data:text/plain;charset=utf-8,' + encodeURIComponent(text));
            element.setAttribute('download', filename);
            element.style.display = 'none';
            document.body.appendChild(element);
            element.click();
            document.body.removeChild(element);
        }

        private openFromFile(files:FileList|null)
        {
            if(files==null || files.length!=1) return;
            const reader = new FileReader();
            reader.onloadend =(e)=>{
                let s:string= <string>e.target!.result;
                let data =<FlowchartData>JSON.parse(s);
                this.setData(data);
            }
            reader.readAsText(files[0]);

        }

        constructor(private container: HTMLDivElement, private options: FlowchartOptions) {
            if(!this.container)
            {
                throw new Error("container is null");
            }
            let subcontainer = <HTMLDivElement>Flowchart.Html(this.container, "div", [], ["develop-ui"]);
            let fileInput = <HTMLInputElement>Flowchart.Html(subcontainer, "input", ["type", "file", "id", "fileInput", "accept", ".json"]);
            fileInput.style.display="none";
            fileInput.onchange=(e)=>{  
                this.openFromFile(fileInput.files);
            }
            
            subcontainer.onclick=(e)=>
            {
                if((<HTMLElement>e.target).classList.contains("dropbtn")) return;
                Array.prototype.forEach.call(document.getElementsByClassName("dropdown-content"), (elem:HTMLDivElement)=>{elem.classList.remove("show");});
            }

            let toolbar = Flowchart.Html(subcontainer, "div", [], ["develop-toolbar"]);
            let menuFile = Flowchart.Html(toolbar, "div", [], ["dropdown"]);
            let menuFileDropBtn = <HTMLButtonElement>Flowchart.Html(menuFile, "button", [], ["dropbtn"], "File ▼");
            let menuFileDropContent = Flowchart.Html(menuFile, "div", [], ["dropdown-content"]);
            menuFileDropBtn.onclick=(e)=>{menuFileDropContent.classList.toggle("show");};
            Flowchart.Html(menuFileDropContent, "a", ["href", "#"], [], "📂 Open").onclick=(e)=>
            {
                Array.prototype.forEach.call(document.getElementsByClassName("dropdown-content"), (elem:HTMLDivElement)=>{elem.classList.remove("show");});
                fileInput.click();
                e.preventDefault();
            }
            Flowchart.Html(menuFileDropContent, "a", ["href", "#"], [], "💾 Save").onclick=(e)=>
            {
                Array.prototype.forEach.call(document.getElementsByClassName("dropdown-content"), (elem:HTMLDivElement)=>{elem.classList.remove("show");});
                this.saveToFile();
                e.preventDefault();
            }
            let runbutton = Flowchart.Html(toolbar, "a", ["href", "#"], ["develop-toolbar"], "Run");
            
            let menuDebug= Flowchart.Html(toolbar, "div", [], ["dropdown"]);
            let menuDebugDropBtn = <HTMLButtonElement>Flowchart.Html(menuDebug, "button", [], ["dropbtn"], "Debug ▼");
            
            let menuDebugDropContent = Flowchart.Html(menuDebug, "div", [], ["dropdown-content"]);
            menuDebugDropBtn.onclick=(e)=>{
                menuDebugDropContent.classList.toggle("show");
            };
            Flowchart.Html(menuDebugDropContent, "a", ["href", "#"], [], "☭ Compile and Run").onclick=(e)=>
            {
                Array.prototype.forEach.call(document.getElementsByClassName("dropdown-content"), (elem:HTMLDivElement)=>{elem.classList.remove("show");});
                this.compile();
                e.preventDefault();
            }
            Flowchart.Html(menuDebugDropContent, "a", ["href", "#"], [], "👣 Run on device").onclick=(e)=>
            {

            }
            let menuDebugLink2 = Flowchart.Html(menuDebugDropContent, "a", ["href", "#"], [], "◉ Stop");
            let menuDebugLink3 = Flowchart.Html(menuDebugDropContent, "a", ["href", "#"], [], "◯ Erase");
            
            
            let workspace = <HTMLDivElement>Flowchart.Html(subcontainer, "div", ["tabindex", "0"], ["develop-workspace"]);//tabindex, damit keypress-Events abgefangen werden können
            this.propertyGridHtmlDiv = <HTMLDivElement>Flowchart.Html(subcontainer, "div", [], ["develop-properties"]);

            

            this.flowchartContainerSvgSvg = <SVGSVGElement>Flowchart.Svg(workspace, "svg", ["width", "100%", "height", "100%"], ["flowchart-container"]);
            
            
            this.linksLayer = <SVGGElement>Flowchart.Svg(this.flowchartContainerSvgSvg, "g", [], ["flowchart-links-layer"]);
            this.operatorsLayer=<SVGGElement>Flowchart.Svg(this.flowchartContainerSvgSvg, "g", [], ["flowchart-operators-layer", "unselectable"]);
            this.tempLayer = <SVGSVGElement>Flowchart.Svg(this.flowchartContainerSvgSvg, "g", [], ["flowchart-temporary-link-layer"]);
            this.tempLayer.style.visibility="hidden";//visible
            let defs = Flowchart.Svg(this.tempLayer, "defs", []);
            let markerArrow = Flowchart.Svg(defs, "marker", ["id", "marker-arrow","markerWidth","4", "markerHeight", "4", "refX", "1", "refY", "2", "orient", "0"]);
            this.markerArrow=<SVGPathElement>Flowchart.Svg(markerArrow, "path", ["d", "M0,0 L0,4 L2,2 z", "fill", "red", "stroke", "black", "stroke-width", "0.5"]);
            let markerCircle = Flowchart.Svg(defs, "marker", ["id", "marker-circle","markerWidth","4", "markerHeight", "4", "refX", "2", "refY", "2", "orient", "0"]);
            this.markerCircle=<SVGCircleElement>Flowchart.Svg(markerCircle, "circle", ["cx", "2", "cy", "2", "r", "2", "fill", "red", "stroke-width", "1px","stroke", "black"]);
            this.temporaryLink = <SVGLineElement>Flowchart.Svg(this.tempLayer, "line", ["x1", "0","y1", "0","x2", "0","y2", "0","stroke-dasharray", "6,6","stroke-width", "4","stroke", "black","fill", "none", "marker-end", "url(#marker-arrow)"]);
            
            let toolsActivator = <SVGRectElement>Flowchart.Svg(this.flowchartContainerSvgSvg, "rect", ["width","40", "height", "100%", "fill", "white", "fill-opacity", "0"]);

            this.toolsLayer  = <SVGSVGElement>Flowchart.Svg(this.flowchartContainerSvgSvg, "g", [], ["flowchart-tools-layer", "unselectable"]);
            this.toolsLayer.style.display="none";//visible
            
            let toolsRect= <SVGRectElement>Flowchart.Svg(this.toolsLayer, "rect", ["width","140", "height", "100%", "rx", "10", "ry", "10"], ["tools-container"]);
            
            //The onmousemove event occurs every time the mouse pointer is moved over the div element.
            //The mouseenter event only occurs when the mouse pointer enters the div element.
            //The onmouseover event occurs when the mouse pointer enters the div element, and its child elements (p and span).

            //The mouseout event triggers when the mouse pointer leaves any child elements as well the selected element.
            //The mouseleave event is only triggered when the mouse pointer leaves the selected element.
            toolsActivator.onmouseenter= (e)=>
            {
                this.toolsLayer.style.display="initial";
            }
            this.toolsLayer.onmouseleave = (e)=>
            {
                this.toolsLayer.style.display="none";
            }

            this.flowchartContainerSvgSvg.onclick = (e) => {
                if (e.target == this.Element)//if the click is in a "free" area, then the target is the uppermost layer; the linkLayer!
                {
                    this.unselectOperator();
                    this.unselectLink();
                }
            }
            this.flowchartContainerSvgSvg.onmouseup=(e)=>
            {
                console.log("Flowchart this.element.onmouseup with e.target="+e.target);
            }

            workspace.onkeyup=(e)=>
            {
                if(e.key=="Delete")
                {
                    console.log("Flowchart workspace.onkeyup with e.target="+e.target+" und Delete-Key");
                    this.deleteSelectedThing();
                }
                else{
                    console.log("Flowchart workspace.onkeyup with e.target="+e.target+" und key "+e.key);
                }
            }

            this.populateToolsLayer();

        }
        get Options() { return this.options; }
        get Element(){return this.flowchartContainerSvgSvg;}

        public onFirstStart()
        {
            if (typeof this.options.data !== undefined && this.options.data!=null) {
                this.setData(this.options.data);
            }
        }

        private populateToolsLayer()
        {
            let y=10;
            for(let clazz in operatorimpl)
            {
                let toolGroup = <SVGGElement>Flowchart.Svg(this.toolsLayer, "g", ["transform", `translate(5 ${y})`]);
                let box = <SVGRectElement>Flowchart.Svg(toolGroup, "rect", ["width","130", "height", "20", "rx", "5", "ry", "5"], ["tool-box"]);
                let title = Flowchart.Svg(toolGroup,"text", ["x", "5", "y", "15"],["tool-caption"]);
            
                toolGroup.onmousedown=(e)=>
                {
                    let type = clazz.substring(0, clazz.length-"Operator".length);
                    let caption = type+"_"+this.upcounter
                    this.upcounter++;
                    let o = this.createOperatorInternal(type, caption, null);
                    let coords = Utils.EventCoordinatesInSVG(e, this.Element);
                    o.MoveTo(coords.x-10, coords.y-10);
                    o.RegisterDragging(e);
                    this.operators.set(o.GlobalOperatorIndex, o);
                }
                title.textContent = clazz;
                y+=25;
            }
        }

        private createOperatorInternal(type:string, caption:string, configurationData:KeyValueTuple[]|null):FlowchartOperator{
            let name = type+"Operator";
            if (!(<any>operatorimpl)[name]) {
                throw new Error(`Unknown type ${type}`);
            }
            if (this.options.onOperatorCreate && !this.options.onOperatorCreate(caption, null, false)) {
                throw new Error(`Creation of operator ${type} prevented by onOperatorCreate plugin`);
            }
            return new (<any>operatorimpl)[name](this, caption, configurationData);
        }

        public setData(data: FlowchartData) {

            this.links.forEach((e)=>e.RemoveFromDOM());
            this.links.clear();
            this.operators.forEach((e)=>e.RemoveFromDOM());
            this.operators.clear();
            let indexInData2operator = new Map<number, FlowchartOperator>();

            for (const d of data.operators) {
                let o= this.createOperatorInternal(d.typeName, d.caption, d.configurationData);
                o.MoveTo(d.posX, d.posY);
                this.operators.set(o.GlobalOperatorIndex,o);
                indexInData2operator.set(d.index, o);
            }
            for (const d of data.links) {
                let fromOp = indexInData2operator.get(d.fromOperatorIndex);
                let toOp = indexInData2operator.get(d.toOperatorIndex);
                if(fromOp===undefined || toOp===undefined) continue;
                let fromConn = fromOp.GetOutputConnectorByIndex(d.fromOutput);
                let toConn = toOp.GetInputConnectorByIndex(d.toInput);
                if(fromConn==null || toConn==null) continue;
                this.createLink(d, fromConn, toConn);
            }
        }
        
        public DeleteLink(globalLinkIndex: number) {
            let l = this.links.get(globalLinkIndex);
            if(l===undefined)
            {
                throw Error("Link to delete is undefined")
            }
            if(this.selectedLink==l)
            {
                this.unselectLink();
            }
            l.RemoveFromDOM();
            this.links.delete(globalLinkIndex);
            l.To.RemoveLink(l);
            l.From.RemoveLink(l);
        }

        public DeleteOperator(globalOperatorIndex:number)
        {
            let o = this.operators.get(globalOperatorIndex);
            if(o===undefined)
            {
                throw Error("Operator to delete is undefined")
            }
            if(this.selectedOperator==o)
            {
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

        public createLink(data: LinkData|null, from: FlowchartOutputConnector, to: FlowchartInputConnector): FlowchartLink|null {
            if (this.options.onLinkCreate && !this.options.onLinkCreate(from.Caption, data)) return null;
            if (!this.options.multipleLinksOnOutput && from.LinksLength > 0) return null;
            if (!this.options.multipleLinksOnInput && to.LinksLength > 0)  return null;

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

        private setTemporaryLink(c:FlowchartOutputConnector) {
            this.lastOutputConnectorClicked = c;
            let color = Flowchart.DATATYPE2COLOR.get(c.Type)
            if(!color) color="BLACK";
            this.markerArrow.style.fill=color;
            this.markerCircle.style.fill=color;
            this.tempLayer.style.visibility = "visible";
        }

        private unselectOperator() {
            if (this.options.onOperatorUnselect && !this.options.onOperatorUnselect()) return;
            this.propertyGridHtmlDiv.innerText=""; //clear
            if (this.selectedOperator == null) return;
            this.selectedOperator.ShowAsSelected(false);
            this.selectedOperator = null;
        }


        public SelectOperator(operator: FlowchartOperator):void {
            if (this.options.onOperatorSelect && !this.options.onOperatorSelect(operator.Caption)) return;
            this.unselectLink();
            if(this.selectedOperator!=null) this.selectedOperator.ShowAsSelected(false);
            operator.ShowAsSelected(true);
            this.selectedOperator = operator;
            this.propertyGridHtmlDiv.innerText=""; //clear
            Flowchart.Html(this.propertyGridHtmlDiv, "p", [],["develop-propertygrid-head"], `Properties for ${this.selectedOperator.Caption}`);
            let table=<HTMLTableElement>Flowchart.Html(this.propertyGridHtmlDiv, "table", [],["develop-propertygrid-table"]);
            let tr=Flowchart.Html(table, "tr", [],["develop-propertygrid-tr"]);
            Flowchart.Html(tr, "th", [],["develop-propertygrid-th"], "Key");
            Flowchart.Html(tr, "th", [],["develop-propertygrid-th"], "Value");
            if(this.selectedOperator!.PopulateProperyGrid(table))
            {
                Flowchart.Html(this.propertyGridHtmlDiv, "button", [],["develop-propertygrid-button"], `Save`);
            }
            else{
                this.propertyGridHtmlDiv.innerText=""; //clear
                Flowchart.Html(this.propertyGridHtmlDiv, "p", [],["develop-propertygrid-head"], `No Properties for ${this.selectedOperator.Caption}`);
            } 
        }

        // Found here : http://stackoverflow.com/questions/5560248/programmatically-lighten-or-darken-a-hex-color-or-rgb-and-blend-colors
        public static _shadeColor(color: string, percent: number) {
            var f = parseInt(color.slice(1), 16), t = percent < 0 ? 0 : 255, p = percent < 0 ? percent * -1 : percent, R = f >> 16, G = f >> 8 & 0x00FF, B = f & 0x0000FF;
            return "#" + (0x1000000 + (Math.round((t - R) * p) + R) * 0x10000 + (Math.round((t - G) * p) + G) * 0x100 + (Math.round((t - B) * p) + B)).toString(16).slice(1);
        }

        public static Svg(parent: Element, type:string,  attributes:string[], classes?: string[]):SVGElement {
            return <SVGElement>Flowchart.Elem(Flowchart.SVGNS, parent, type, attributes, classes);
        }

        public static Html(parent: Element, type:string,  attributes:string[], classes?: string[], textContent?:string):HTMLElement {
            return <HTMLElement>Flowchart.Elem(Flowchart.HTMLNS, parent, type, attributes, classes, textContent);
        }

        private static Elem(ns:string, parent:Element, type:string, attributes:string[], classes?: string[], textContent?:string)
        {
            let element = document.createElementNS(ns, type);
            if(classes)
            {
                for (const clazz of classes) {
                    element.classList.add(clazz);
                }
            }
            let i:number;
            for(i=0;i<attributes.length;i+=2)
            {
                element.setAttribute(attributes[i], attributes[i+1]);
            }
            if(textContent)
            {
                element.textContent=textContent;
            }
            parent.appendChild(element);
            return element;
        }
    }
