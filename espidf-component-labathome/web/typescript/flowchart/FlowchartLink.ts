import { FlowchartInputConnector, FlowchartOutputConnector } from "./FlowchartConnector";
import { Flowchart } from "./Flowchart";
import { Svg, XLINKNS } from "../utils/common";

export class FlowchartLink {
    private static MAX_INDEX: number = 0;
    private index: number;
    private element: SVGPathElement;
    private captionElement:SVGTextElement;
    private captionPath: SVGTextPathElement;
    constructor(private parent: Flowchart, private caption: string, private color: string, private from: FlowchartOutputConnector, private to: FlowchartInputConnector) {
        this.index = FlowchartLink.MAX_INDEX++;
        this.element = <SVGPathElement>Svg(parent.LinkLayer, "path", ["stroke-width", "" + this.parent.Options.linkWidth, "fill", "none", "id", "LINK" + this.index]);
        this.RefreshPosition();
        this.UnsetColor();
        this.parent.LinkLayer.appendChild(this.element);
        this.captionElement =<SVGTextElement>Svg(parent.LinkLayer, "text",[]);
        this.captionPath = <SVGTextPathElement>Svg(this.captionElement, "textPath",["startOffset", "50%","text-anchor", "middle"]);
        this.captionPath.setAttributeNS(XLINKNS, "href", '#' + "LINK" + this.index);
        this.captionPath.innerHTML = caption;
        this.element.onclick = (e) => {
            this.parent._notifyLinkClicked(this, e);
        }
    }
    get GlobalLinkIndex() { return this.index; }
    get From() { return this.from; }
    get To() { return this.to; }
    set Color(color: string) { this.color = color; }

    public RemoveFromDOM() {
        this.element.remove();
        this.captionElement.remove();
    }


    public SetColor(color: string) {
        this.element.setAttribute('stroke', color);
        //this.element.setAttribute('fill', color);
        //TODO: colorize the small triangle in the connector
        //linkData.internal.els.fromSmallConnector.css('border-left-color', color);
        //linkData.internal.els.toSmallConnector.css('border-left-color', color);
    }

    public SetCaption(caption:string){
        this.captionPath.innerHTML=caption;
    }

    public UnsetColor() {
        this.SetColor(this.parent.Options.defaultLinkColor);
    }

    public RefreshPosition() {
        let fromPosition = this.from.GetLinkpoint();
        let toPosition = this.to.GetLinkpoint();
        let fromX = fromPosition.x;
        let fromY = fromPosition.y + this.parent.Options.linkVerticalDecal;
        let toX = toPosition.x;
        let toY = toPosition.y + this.parent.Options.linkVerticalDecal;
        let distanceFromArrow = this.parent.Options.distanceFromArrow;
        let bezierFromX = (fromX + distanceFromArrow);
        let bezierToX = toX + 1;
        let bezierIntensity = Math.min(100, Math.max(Math.abs(bezierFromX - bezierToX) / 2, Math.abs(fromY - toY)));
        this.element.setAttribute("d", 'M' + bezierFromX + ',' + (fromY) + ' C' + (fromX + distanceFromArrow + bezierIntensity) + ',' + fromY + ' ' + (toX - bezierIntensity) + ',' + toY + ' ' + bezierToX + ',' + toY);
    }
}
