import {FlowchartInputConnector, FlowchartOutputConnector } from "./flowchartConnector";
import {Flowchart} from "./flowchart";
import {Location2D, Utils} from "./utils"
export abstract class FlowchartOperator {

    private Inputs: FlowchartInputConnector[]=[];
    private Outputs: FlowchartOutputConnector[]= [];


    private static INDEX: number = 0;
    private index: number;
    public GetIndex = () => this.index;

    private elementSvgG: SVGGElement;
    get ElementSvgG() { return this.elementSvgG; }
    private inputSvgG:SVGGElement;
    get InputSvgG(): SVGGElement { return this.inputSvgG; }
    private outputSvgG:SVGGElement;
    get OutputSvgG(): SVGGElement { return this.outputSvgG;}

    private x=0;
    private y=0;

    private box:SVGRectElement;
    constructor(private parent: Flowchart, private type: string, private caption: string) {
        this.index = FlowchartOperator.INDEX++;
        this.elementSvgG = <SVGGElement>Flowchart.Svg(parent.OperatorsLayer, "g", [], ["operator"]);

        this.elementSvgG.setAttribute('data-operator-index', "" + this.index);
        this.box = <SVGRectElement>Flowchart.Svg(this.elementSvgG, "rect", ["width","140", "height", "100", "rx", "10", "ry", "10"], ["operator-box"]);
        let title = Flowchart.Svg(this.elementSvgG,"text", ["x", "5", "y", "21"],["operator-title"]);
        title.textContent = caption;

        
        this.inputSvgG= <SVGGElement>Flowchart.Svg(this.elementSvgG,"g", ["transform", "translate(0 50)"], ["operator-inputs"]);
        this.outputSvgG= <SVGGElement>Flowchart.Svg(this.elementSvgG,"g", ["transform", "translate(140 50)"], ["operator-outputs"]);
       

        this.elementSvgG.onclick = (e) => { parent._notifyOperatorClicked(this, e) };
        if (this.parent.Options.canUserMoveOperators) {
            title.onmousedown = (e) => {
                this.RegisterDragging(e);
            }
        }
    }
    public RegisterDragging(e:MouseEvent)
    {
        let offsetInOperator = Utils.EventCoordinatesInSVG(e, this.ElementSvgG); //offset innerhalb des Operators
        //Wir benötigen den Offset zwischen der aktuellen Position des Objektes und 
        let offsetX= e.clientX-this.x;
        let offsetY = e.clientY-this.y;


        document.onmouseup = (e) => {
            document.onmouseup = null;
            document.onmousemove = null;
        };
        document.onmousemove = e => {
            //TODO: neue Position nur setzen, wenn this.element.clientRect innerhalb von parent.clientRectangle ist
            this.MoveTo(e.clientX - offsetX, e.clientY - offsetY);
        };
    }
    get Parent() { return this.parent };
    get Type() { return this.type };
    get Caption() { return this.caption; }

    get InputsKVIt(){return this.Inputs.entries()}
    get OutputsKVIt(){return this.Outputs.entries()}
    public GetOutputConnectorByIndex=(i:number)=>this.Outputs[i];
    public GetInputConnectorByIndex=(i:number)=>this.Inputs[i];

    public Dispose(): void {
        this.elementSvgG.remove();
    }


    protected AppendConnectors(inputs: FlowchartInputConnector[], outputs: FlowchartOutputConnector[]) {
        if(this.Inputs.length!=0 || this.Outputs.length !=0) throw new Error("AppendConnectors may only be called once!");
        for (const i of inputs) {
            if (i.Parent != this) continue;
            this.Inputs.push(i);
        }
        for (const o of outputs) {
            if (o.Parent != this) continue;
            this.Outputs.push(o);
        }
        let num = Math.max(this.Inputs.length, this.Outputs.length);
        let height = 50+num*20+10;
        this.box.setAttribute("height", ""+height);
        //TODO RedrawConnectors; Connectors zeichnen sich nicht im Construktur, sondern erst nach dem Appenden, um die Reihenfolgen in derser Liste und im DOM gleich zu haben
    }

    public MoveTo(x: number, y: number) {
        let g = this.parent.Options.grid;
        this.x = Math.round(x / g) * g;
        this.y = Math.round(y / g) * g;
        this.elementSvgG.setAttribute("transform", `translate(${this.x} ${this.y})`);
        for (const c of this.Inputs) {
            c.RefreshLinkPositions();
        }
        for (const c of this.Outputs) {
            c.RefreshLinkPositions();
        }
    }
}
