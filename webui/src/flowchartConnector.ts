import { FlowchartOperator } from "./flowchartOperator";
import { Flowchart, ConnectorType } from "./flowchart";
import { FlowchartLink } from "./flowchartLink";
import {Location2D} from "./utils"

const TRANSLATEY = 20;

export abstract class FlowchartConnector {
    private static INDEX: number = 0;
    private index: number;
    get Index() { return this.index; }

    protected element: SVGGElement;
    get Element() { return this.element; }

    protected snapper:SVGCircleElement;
    protected connector:SVGElement;
    protected connectorGroup:SVGGElement;

    private links: FlowchartLink[] = [];
    public HasLink = (globalLinkIndex: number) => { return this.links[globalLinkIndex] !== undefined && this.links[globalLinkIndex] != null };
    public AddLink = (link: FlowchartLink) => {
        this.links[link.GlobalLinkIndex] = link;
    }
    public RemoveLink = (link: FlowchartLink) => {
        delete this.links[link.GlobalLinkIndex];
    }
    get LinksLength() { return this.links.reduce((sum, item): number => sum + (item != null ? 1 : 0), 0) };
    public GetLinksCopy(): FlowchartLink[] {
        return this.links.map(x => x);
    }

    get LinksKVIt(){return this.links.entries()}
    protected abstract GetLinkpointXOffset(width:number): number;
    protected abstract getIOSpecifics():{inputOrOutput:string, parent:SVGGElement, translateY:number, dx:number};

    public RefreshLinkPositions() {
        this.links.forEach(l=>{
            l.RefreshPosition();
        });
        
    }

    constructor(private parent: FlowchartOperator, private caption: string, private type:ConnectorType) {

        this.index = FlowchartConnector.INDEX++;
        let spec = this.getIOSpecifics();
        let translateY = TRANSLATEY*spec.parent.childElementCount;
        this.element = <SVGGElement>Flowchart.Svg(spec.parent, "g", ["transform", `translate(0 ${translateY})`], [`operator-${spec.inputOrOutput}`]);
        this.element.setAttribute("data-connector-datatype", ConnectorType[type]);

        let text =  <SVGTextElement>Flowchart.Svg(this.element, "text", ["dx",""+spec.dx, "dy", "4"], [`operator-${spec.inputOrOutput}-caption`]);
        text.textContent=caption;
        this.connectorGroup = <SVGGElement>Flowchart.Svg(this.element, "g", []);
        this.connector = <SVGCircleElement>Flowchart.Svg(this.connectorGroup, "circle", ["r","4"], [`operator-${spec.inputOrOutput}-connector`, ConnectorType[type]]);
        this.snapper= <SVGCircleElement>Flowchart.Svg(this.connectorGroup, "circle", ["r","10"], [`operator-${spec.inputOrOutput}-snapper`]);
        
        this.element.onmouseover = (e) => {
            for (const link of this.links) {
                if (link && link != this.parent.Parent.SelectedLink) {
                    link.ColorizeLink(Flowchart._shadeColor(this.parent.Parent.Options.defaultLinkColor, -0.4));
                }
            }
        }

        this.element.onmouseout = (e) => {
            for (const link of this.links) {
                if (link && link != this.parent.Parent.SelectedLink) {
                    link.UncolorizeLink();
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
    constructor (parent: FlowchartOperator, caption: string, type:ConnectorType) {
        super(parent, caption, type);
        
        this.connectorGroup.onmouseup = (e) => {
           parent.Parent._notifyInputConnectorMouseupWithLink(this, e);
        }
        this.connectorGroup.onmouseenter=(e)=>
        {
            parent.Parent._notifySnapStart(this, e);
        }
        this.connectorGroup.onmouseleave=(e)=>
        {
            parent.Parent._notifySnapEnd(this, e);
        }
    }
    protected GetLinkpointXOffset(width:number): number{return 0;}  
    protected  getIOSpecifics(){return {inputOrOutput:"input", parent:this.Parent.InputSvgG, translateY:0, dx:8};}
}
export class FlowchartOutputConnector extends FlowchartConnector {
    constructor (parent: FlowchartOperator, caption: string, type:ConnectorType) {
        super(parent, caption, type)
        this.element.onmousedown = (e) => {
            parent.Parent._notifyOutputConnectorMousedown(this, e);
            document.onmouseup = (e) => {
                document.onmouseup = null;
                document.onmousemove = null;
                parent.Parent._notifyDocumentMouseupWithLink(e);
            };
            document.onmousemove = (e) => {
                parent.Parent._notifyMouseMovedWithLink(e);
            };
        }
    }
    protected GetLinkpointXOffset(width:number): number{return width;}
    protected  getIOSpecifics(){return {inputOrOutput:"output", parent:this.Parent.OutputSvgG, translateY:140, dx:-8};}
}
