import { FlowchartOperator } from "./flowchartOperator";
import { FlowchartLink } from "./flowchartLink";
import { FlowchartInputConnector, FlowchartOutputConnector } from "./flowchartConnector";
import * as operatorimpl from "./flowchartOperatorImpl";
import {Utils} from "./utils"
import "./flowchart.scss";

export class FlowchartOptions {
    canUserEditLinks?: boolean = true;
    canUserMoveOperators?: boolean = true;
    data?: any = null;
    distanceFromArrow?: number = 3;
    defaultOperatorClass?: string = 'flowchart-default-operator';
    defaultLinkColor: string = '#3366ff';
    defaultSelectedLinkColor?: string = 'black';
    linkWidth?: number = 10;
    grid: number = 10;
    multipleLinksOnOutput?: boolean = true;
    multipleLinksOnInput?: boolean = true;
    linkVerticalDecal?: number = 0;
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
        type: string;
        caption: string;
        id: string;
        posX: number;
        posY: number;
    }

    export interface LinkData {
        color: string;
        fromId: string;
        fromOutput: number;
        toId: string;
        toInput: number;
    }

    export enum ConnectorType{
        BOOLEAN,
        INTEGER,
        FLOAT,
    }

    //Connector besteht aus "wrapper", dem Label, dem großen dreieck und einem kleinen Dreieck

    export class Flowchart {
        private operators: FlowchartOperator[] = [];
        private links: FlowchartLink[] = [];
        public static readonly SVGNS = "http://www.w3.org/2000/svg";
        public static readonly XLINKNS = "http://www.w3.org/1999/xlink";
        private lastOutputConnectorClicked: FlowchartOutputConnector = null;
        private selectedOperator: FlowchartOperator = null;
        private selectedLink: FlowchartLink = null;
        get SelectedLink() { return this.selectedLink };
        
        private positionRatio: number = 1;
        get PositionRatio() { return this.positionRatio; }
        
        private element:SVGElement
        private linksLayer: SVGGElement;
        get LinkLayer() { return this.linksLayer; }
        private operatorsLayer: SVGGElement;
        get OperatorsLayer() { return this.operatorsLayer; }
        private toolsLayer:SVGGElement;
        get ToolsLayer() { return this.toolsLayer; }
        private tempLayer:SVGGElement;
        private temporaryLink: SVGLineElement;
        private temporaryLinkSnapped=false;

        private upcounter=0;

        public _notifyOperatorClicked(o: FlowchartOperator, e: MouseEvent) {
            this.SelectOperator(o);
        }

        public _notifyOutputConnectorMousedown(c: FlowchartOutputConnector, e: MouseEvent) {
            this.temporaryLinkSnapped=false;
            let start = c.GetLinkpoint();
            let end = Utils.EventCoordinatesInSVG(e, this.element, this.positionRatio);
            this.temporaryLink.setAttribute('x1', ""+start.x);
            this.temporaryLink.setAttribute('y1', ""+start.y);
            this.temporaryLink.setAttribute('x2', ""+end.x);
            this.temporaryLink.setAttribute('y2', ""+end.y);
            this._setTemporaryLink(c);
        }

        public _notifyDocumentMouseupWithLink(e:MouseEvent)
        {
            this._unsetTemporaryLink();
        }

        public _notifyInputConnectorMouseupWithLink(c: FlowchartInputConnector, e:MouseEvent) {
            if (this.lastOutputConnectorClicked == null) return;
            if(this.lastOutputConnectorClicked.Type==c.Type)
            {
                this.createLink(null, this.lastOutputConnectorClicked, c);
            }
            this._unsetTemporaryLink();

        }

        public _notifyLinkClicked(link: FlowchartLink, e: MouseEvent) {
            this.selectLink(link);
        }

        public _notifySnapStart(c:FlowchartInputConnector, e:MouseEvent){
            console.log("_notifySnapStart");
            this.temporaryLinkSnapped=true;
            let end= c.GetLinkpoint();
            this.temporaryLink.setAttribute('x2', ""+end.x);
            this.temporaryLink.setAttribute('y2', ""+end.y);
        }

        public _notifySnapEnd(c:FlowchartInputConnector, e:MouseEvent){
            console.log("_notifySnapEnd");
            this.temporaryLinkSnapped=false;
        }

        public _notifyMouseMovedWithLink(e:MouseEvent)
        {
            if (this.lastOutputConnectorClicked != null && !this.temporaryLinkSnapped) {
                let end = Utils.EventCoordinatesInSVG(e, this.element, this.positionRatio);
                this.temporaryLink.setAttribute('x2', ""+end.x);
                this.temporaryLink.setAttribute('y2', ""+end.y);
            }
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

        constructor(element: HTMLDivElement, private options: FlowchartOptions) {
            if(!element)
            {
                throw new Error("element is null");
            }
            element.classList.add('flowchart-container');
            this.element = <SVGSVGElement>Flowchart.Svg(element, "svg", ["width", "100%", "height", "100%"], ["flowchart-container"]);
            
            
            this.linksLayer = <SVGGElement>Flowchart.Svg(this.element, "g", [], ["flowchart-links-layer"]);
            this.operatorsLayer=<SVGGElement>Flowchart.Svg(this.element, "g", [], ["flowchart-operators-layer", "unselectable"]);
            this.tempLayer = <SVGSVGElement>Flowchart.Svg(this.element, "g", [], ["flowchart-temporary-link-layer"]);
            this.tempLayer.style.visibility="hidden";//visible
            let defs = Flowchart.Svg(this.tempLayer, "defs", []);
            let markerArrow = Flowchart.Svg(defs, "marker", ["id", "marker-arrow","markerWidth","4", "markerHeight", "4", "refX", "1", "refY", "2", "orient", "0"]);
            Flowchart.Svg(markerArrow, "path", ["d", "M0,0 L0,4 L2,2 z", "fill", "#f00"]);
            let markerCircle = Flowchart.Svg(defs, "marker", ["id", "marker-circle","markerWidth","4", "markerHeight", "4", "refX", "2", "refY", "2", "orient", "0"]);
            Flowchart.Svg(markerCircle, "circle", ["cx", "2", "cy", "2", "r", "2", "fill", "red"]);
            this.temporaryLink = <SVGLineElement>Flowchart.Svg(this.tempLayer, "line", ["x1", "0","y1", "0","x2", "0","y2", "0","stroke-dasharray", "6,6","stroke-width", "4","stroke", "black","fill", "none", "marker-end", "url(#marker-arrow)"]);
            
            let toolsActivator = <SVGRectElement>Flowchart.Svg(this.element, "rect", ["width","40", "height", "100%", "fill", "white", "fill-opacity", "0"]);

            this.toolsLayer  = <SVGSVGElement>Flowchart.Svg(this.element, "g", [], ["flowchart-tools-layer", "unselectable"]);
            this.toolsLayer.style.display="none";//visible
            
            let toolsRect= <SVGRectElement>Flowchart.Svg(this.toolsLayer, "rect", ["width","140", "height", "100%", "rx", "10", "ry", "10"], ["tools-container"]);
            toolsActivator.onmousedown=(e)=>{
                console.log("toolsActivator.onmousedown");
            }
            //The onmousemove event occurs every time the mouse pointer is moved over the div element.
            //The mouseenter event only occurs when the mouse pointer enters the div element.
            //The onmouseover event occurs when the mouse pointer enters the div element, and its child elements (p and span).

            //The mouseout event triggers when the mouse pointer leaves any child elements as well the selected element.
            //The mouseleave event is only triggered when the mouse pointer leaves the selected element.
            toolsActivator.onmouseenter= (e)=>//
            {
                this.toolsLayer.style.display="initial";
            }
            this.toolsLayer.onmouseleave = (e)=>
            {
                this.toolsLayer.style.display="none";
            }

            this.element.onclick = (e) => {
                if (e.target == this.Element)//if the click is in a "free" area, then the target is the uppermost layer; the linkLayer!
                {
                    this.unselectOperator();
                    this.unselectLink();
                }
            }
            this.element.onmouseup=(e)=>
            {
                console.log("this.element.onmouseup with e.target="+e.target);
            }

            this.populateToolsLayer();

            if (typeof this.options.data !== undefined && this.options.data!=null) {
                this.setData(this.options.data);
            }
        }
        get Options() { return this.options; }
        get Element(){return this.element;}

        private populateToolsLayer()
        {
            let y=10;
            for(let clazz in operatorimpl)
            {
                let toolGroup = <SVGGElement>Flowchart.Svg(this.toolsLayer, "g", ["transform", `translate(5 ${y})`]);
                let box = <SVGRectElement>Flowchart.Svg(toolGroup, "rect", ["width","130", "height", "30", "rx", "10", "ry", "10"], ["tool-box"]);
                let title = Flowchart.Svg(toolGroup,"text", ["x", "5", "y", "25"],["tool-caption"]);
                toolGroup.onmousedown=(e)=>
                {
                    
                    let cnt = this.upcounter;
                    let name = clazz.substring(0, clazz.length-"Operator".length)+"_"+this.upcounter
                    if (this.options.onOperatorCreate && !this.options.onOperatorCreate(name, null, false)) {
                        return null;
                    }
                    console.log("Creating "+name);
                    this.upcounter++;
                    let o: FlowchartOperator = new (<any>operatorimpl)[clazz](this, name);
                    let coords = Utils.EventCoordinatesInSVG(e, this.Element);
                    o.MoveTo(coords.x-10, coords.y-10);
                    o.RegisterDragging(e);
                    this.operators[o.GetIndex()] = o;
                }
                title.textContent = clazz;
                y+=40;
            }
        }

        public setData(data: FlowchartData) {

            for (const id in this.links) {
                let l = this.links[id];
                if (l) l.Dispose();
            }
            this.links = [];

            for (const id in this.operators) {
                let o = this.operators[id];
                if (o) o.Dispose();
            }
            this.operators = [];
            let opId2op: { [id: string]: FlowchartOperator } = {};

            for (const d of data.operators) {
                let o = this.createOperator(d);
                opId2op[d.id] = o;
            }
            for (const d of data.links) {
                let fromOp = opId2op[d.fromId];
                let toOp = opId2op[d.toId];
                let fromConn = fromOp.GetOutputConnectorByIndex(d.fromOutput);
                let toConn = toOp.GetInputConnectorByIndex(d.toInput);
                this.createLink(d, fromConn, toConn);
            }
        }
        public DeleteLink(globalLinkIndex: number) {
            let l = this.links[globalLinkIndex];
            this.links[globalLinkIndex] = null;
            l.To.RemoveLink(l);
            l.From.RemoveLink(l);
        }

        public createLink(data: LinkData, from: FlowchartOutputConnector, to: FlowchartInputConnector): FlowchartLink {
            if (this.options.onLinkCreate && !this.options.onLinkCreate(from.Caption, null)) return;

            if (!this.options.multipleLinksOnOutput) {
                if (from.LinksLength > 0) {
                    for (const link of from.GetLinksCopy()) {
                        if (!link) continue;
                        this.DeleteLink(link.GlobalLinkIndex);
                    }
                }
            }
            if (!this.options.multipleLinksOnInput) {
                if (to.LinksLength > 0) {
                    for (const link of to.GetLinksCopy()) {
                        if (!link) continue;
                        this.DeleteLink(link.GlobalLinkIndex);
                    }
                }
            }
            let l: FlowchartLink = new FlowchartLink(this, "", this.Options.defaultLinkColor, from, to);
            from.AddLink(l);
            to.AddLink(l);
            this.links[l.GlobalLinkIndex] = l;
        }
       
        public createOperator(data: OperatorData): FlowchartOperator {

            let name = data.type+"Operator";
            if (!(<any>operatorimpl)[name]) {
                throw new Error("Unknown type " + data.type);
            }
            if (this.options.onOperatorCreate && !this.options.onOperatorCreate(data.caption, null, false)) {
                return null;
            }
            let o: FlowchartOperator = new (<any>operatorimpl)[name](this, data.caption);
            o.MoveTo(data.posX, data.posY);
            this.operators[o.GetIndex()] = o;
            return o;
        }

        private _unsetTemporaryLink() {
            this.lastOutputConnectorClicked = null;
            this.tempLayer.style.visibility = "hidden";
        }

        private _setTemporaryLink(c:FlowchartOutputConnector) {
            this.lastOutputConnectorClicked = c;
            let marker:string;
            switch (c.Type)
            {
                case ConnectorType.BOOLEAN:marker="marker-arrow";break;
                case ConnectorType.INTEGER:marker="marker-circle";break;
                case ConnectorType.FLOAT:marker="marker-rect";break;
                
            }
            this.temporaryLink.setAttribute("marker-end", "url(#"+marker+")");
            this.tempLayer.style.visibility = "visible";
        }

        private _removeSelectedClassOperators(operator: FlowchartOperator) {
            if (operator != null) {
                operator.ElementSvgG.classList.remove('selected');
            }
        }

        private _addSelectedClass(operator: FlowchartOperator) {
            operator.ElementSvgG.classList.add('selected');
        }


        private unselectOperator() {
            if (this.selectedOperator != null) {
                if (this.options.onOperatorUnselect && !this.options.onOperatorUnselect()) return;
                this._removeSelectedClassOperators(this.selectedOperator);
                this.selectedOperator = null;
            }
        }




        public SelectOperator(operator: FlowchartOperator) {
            if (this.options.onOperatorSelect && !this.options.onOperatorSelect(operator.Caption)) return;
            this.unselectLink();
            this._removeSelectedClassOperators(this.selectedOperator);
            this._addSelectedClass(operator);
            this.selectedOperator = operator;
        }



        // Found here : http://stackoverflow.com/questions/5560248/programmatically-lighten-or-darken-a-hex-color-or-rgb-and-blend-colors
        public static _shadeColor(color: string, percent: number) {
            var f = parseInt(color.slice(1), 16), t = percent < 0 ? 0 : 255, p = percent < 0 ? percent * -1 : percent, R = f >> 16, G = f >> 8 & 0x00FF, B = f & 0x0000FF;
            return "#" + (0x1000000 + (Math.round((t - R) * p) + R) * 0x10000 + (Math.round((t - G) * p) + G) * 0x100 + (Math.round((t - B) * p) + B)).toString(16).slice(1);
        }

        public static Svg(parent: Element, type:string,  attributes:string[], classes?: string[]) {
            let element = document.createElementNS(Flowchart.SVGNS, type);
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
            parent.appendChild(element);
            return element;
        }
    }
