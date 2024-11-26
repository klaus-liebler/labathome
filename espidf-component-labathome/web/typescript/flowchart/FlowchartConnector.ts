import { FlowchartOperator } from "./FlowchartOperator";
import { Flowchart } from "./Flowchart";
import { FlowchartLink } from "./FlowchartLink";
import {Location2D, Svg} from "../utils/common"

const TRANSLATEY = 20;

export enum ConnectorType{
    BOOLEAN=0,
    INTEGER=1,
    FLOAT=2,
    COLOR=3,
}

export abstract class FlowchartConnector {
    private static INDEX: number = 0;
    private globalConnectorIndex: number;
    get GlobalConnectorIndex() { return this.globalConnectorIndex; }
    get LocalConnectorIndex(){return this.localIndex;}

    protected element: SVGGElement;
    get Element() { return this.element; }

    protected snapper:SVGCircleElement;
    protected connector:SVGElement;
    protected connectorGroup:SVGGElement;

    protected  links = new Map<number, FlowchartLink>();
    public HasLink = (globalLinkIndex: number) => this.links.has(globalLinkIndex);
    public AddLink = (link: FlowchartLink) => this.links.set(link.GlobalLinkIndex, link);
    public RemoveLink = (link: FlowchartLink) => this.links.delete(link.GlobalLinkIndex);
    get LinksLength() { return this.links.size};
    public GetLinksCopy(): FlowchartLink[] {
        return Array.from(this.links.values());
    }

    get LinksKVIt(){return this.links.entries()}
    protected abstract GetLinkpointXOffset(width:number): number;
    protected abstract getIOSpecifics():{inputOrOutput:string, parent:SVGGElement, translateY:number, dx:number};

    public RefreshLinkPositions() {
        this.links.forEach(l=>{
            l.RefreshPosition();
        });
        
    }

    constructor(private parent: FlowchartOperator, private caption: string, private localIndex:number, private type:ConnectorType) {

        this.globalConnectorIndex = FlowchartConnector.INDEX++;
        let spec = this.getIOSpecifics();
        let translateY = TRANSLATEY*spec.parent.childElementCount;
        this.element = <SVGGElement>Svg(spec.parent, "g", ["transform", `translate(0 ${translateY})`], [`operator-${spec.inputOrOutput}`]);
        this.element.setAttribute("data-connector-datatype", ConnectorType[type]);

        let text =  <SVGTextElement>Svg(this.element, "text", ["dx",""+spec.dx, "dy", "4"], [`operator-${spec.inputOrOutput}-caption`]);
        text.textContent=caption;
        this.connectorGroup = <SVGGElement>Svg(this.element, "g", []);
        this.connector = <SVGCircleElement>Svg(this.connectorGroup, "circle", ["r","4"], [`operator-${spec.inputOrOutput}-connector`, ConnectorType[type]]);
        this.snapper= <SVGCircleElement>Svg(this.connectorGroup, "circle", ["r","10"], [`operator-${spec.inputOrOutput}-snapper`]);
        
        this.element.onmouseover = (e) => {
            for (const link of this.links.values()) {
                if (link && link != this.parent.Parent.SelectedLink) {
                    link.SetColor(Flowchart._shadeColor(this.parent.Parent.Options.defaultLinkColor, -0.4));
                }
            }
        }

        this.element.onmouseout = (e) => {
            for (const link of this.links.values()) {
                if (link && link != this.parent.Parent.SelectedLink) {
                    link.UnsetColor();
                }
            }
        }
    }
    get Parent() { return this.parent; }
    get Caption() { return this.caption; }
    get Type() { return this.type; }


    public GetLinkpoint(): Location2D {
        let flowchart = this.Parent.Parent;
        let posrat = flowchart.PositionRatio;
        let flowchartRect = flowchart.Element.getBoundingClientRect();
        let connectorRect = this.connector.getBoundingClientRect();
        var x = (connectorRect.left - flowchartRect.left) / posrat + connectorRect.width/2;
        var y = (connectorRect.top - flowchartRect.top) / posrat + connectorRect.height/2;
        return { x: x, y: y };
    }
}

export class FlowchartInputConnector extends FlowchartConnector {
    constructor (parent: FlowchartOperator, caption: string, localIndex:number, type:ConnectorType) {
        super(parent, caption, localIndex, type);
        
        this.connectorGroup.onmouseup = (e) => {
           parent.Parent._notifyInputConnectorMouseup(this, e);
        }
        this.connectorGroup.onmouseenter=(e)=>
        {
            parent.Parent._notifyInputConnectorMouseenter(this, e);
        }
        this.connectorGroup.onmouseleave=(e)=>
        {
            parent.Parent._notifyInputConnectorMouseleave(this, e);
        }
    }
    protected GetLinkpointXOffset(width:number): number{return 0;}  
    protected  getIOSpecifics(){return {inputOrOutput:"input", parent:this.Parent.InputSvgG, translateY:0, dx:8};}
    public GetGlobalConnectorIndexOfSignalSource():number {
        for(let link of this.links.values()){
            return link.From.GlobalConnectorIndex;
        }
        return -1;
    }
}
export class FlowchartOutputConnector extends FlowchartConnector {
    constructor (parent: FlowchartOperator, caption: string, localIndex:number, type:ConnectorType) {
        super(parent, caption, localIndex, type)
        this.element.onmousedown = (e) => {
            parent.Parent._notifyOutputConnectorMousedown(this, e);
        }
    }
    protected GetLinkpointXOffset(width:number): number{return width;}
    protected  getIOSpecifics(){return {inputOrOutput:"output", parent:this.Parent.OutputSvgG, translateY:140, dx:-8};}
}
