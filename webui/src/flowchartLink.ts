import { FlowchartInputConnector, FlowchartOutputConnector } from "./flowchartConnector";
import { Flowchart } from "./flowchart";
export class FlowchartLink {
    private static INDEX: number = 0;
    private index: number;
    private element: SVGPathElement;
    private captionElement: SVGTextElement;
    constructor(private parent: Flowchart, private caption: string, private color: string, private from: FlowchartOutputConnector, private to: FlowchartInputConnector) {
        this.index = FlowchartLink.INDEX++;
        this.element = <SVGPathElement>Flowchart.Svg(parent.LinkLayer, "path", ["stroke-width", "" + this.parent.Options.linkWidth, "fill", "none", "id", "LINK" + this.index]);
        this.RefreshPosition();
        this.UncolorizeLink();
        this.parent.LinkLayer.appendChild(this.element);
        this.captionElement =<SVGTextElement>Flowchart.Svg(parent.LinkLayer, "text",[]);
        let captionPath = <SVGTextPathElement>Flowchart.Svg(this.captionElement, "textPath",["startOffset", "50%","text-anchor", "middle"]);
        captionPath.setAttributeNS(Flowchart.XLINKNS, "href", '#' + "LINK" + this.index);
        captionPath.innerHTML = caption;
        this.element.onclick = (e) => {
            this.parent._notifyLinkClicked(this, e);
        }
    }
    get GlobalLinkIndex() { return this.index; }
    get From() { return this.from; }
    get To() { return this.to; }
    set Color(color: string) { this.color = color; }

    public Dispose() {
        this.element.remove();
        this.captionElement.remove();
    }


    public ColorizeLink(color: string) {
        this.element.setAttribute('stroke', color);
        //this.element.setAttribute('fill', color);
        //TODO: colorize the small triangle in the connector
        //linkData.internal.els.fromSmallConnector.css('border-left-color', color);
        //linkData.internal.els.toSmallConnector.css('border-left-color', color);
    }

    public UncolorizeLink() {
        this.ColorizeLink(this.parent.Options.defaultLinkColor);
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
