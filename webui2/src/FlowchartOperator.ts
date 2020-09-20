import {ConnectorType, FlowchartInputConnector, FlowchartOutputConnector } from "./FlowchartConnector";
import {Flowchart} from "./Flowchart";
import {Utils} from "./Utils"
import { SerializeContext } from "./FlowchartExporter";

export enum PositionType{
    Default,
    Input,
    Output,
};
export enum SingletonType{
    Default,
    Singleton,
};
export class TypeInfo
{
    constructor(public GlobalTypeIndex:number, public Position:PositionType, public Singleton:SingletonType){}
}

export abstract class FlowchartOperator {

    //der Index der Inputs ist rein lokal und beginnt bei 0 fortlaufend
    private Inputs: FlowchartInputConnector[]=[];
    //der Index der Outputs ist rein lokal und beginnt bei 0 fortlaufend
    private Outputs: FlowchartOutputConnector[]= [];


    private static MAX_INDEX: number = 0;
    private index: number;
    get GlobalOperatorIndex(){return this.index;}

    private elementSvgG: SVGGElement;
    get ElementSvgG() { return this.elementSvgG; }
    private inputSvgG:SVGGElement;
    get InputSvgG(): SVGGElement { return this.inputSvgG; }
    private outputSvgG:SVGGElement;
    get OutputSvgG(): SVGGElement { return this.outputSvgG;}
    private debugInfoSvgText:SVGTextElement;

    get TypeInfo(){return this.typeInfo;}

    private x=0;
    private y=0;

    private box:SVGRectElement;

    public ShowAsSelected(state:boolean)
    {
        if(state)
        {
            this.box.classList.add('selected');
        }
        else{
            this.box.classList.remove('selected');
        }
    }

    public SetDebugInfoText(text:string):void{
        this.debugInfoSvgText.textContent=text;
    }

    constructor(private parent: Flowchart, private caption: string, private typeInfo: TypeInfo) {
        this.index = FlowchartOperator.MAX_INDEX++;
        this.elementSvgG = <SVGGElement>Flowchart.Svg(parent.OperatorsLayer, "g", [], ["operator"]);

        this.elementSvgG.setAttribute('data-operator-index', "" + this.index);
        this.box = <SVGRectElement>Flowchart.Svg(this.elementSvgG, "rect", ["width","140", "height", "100", "rx", "10", "ry", "10"], ["operator-box"]);
        let title = <SVGTextElement>Flowchart.Svg(this.elementSvgG,"text", ["x", "5", "y", "21"],["operator-title"]);
        title.textContent = caption;
        this.inputSvgG= <SVGGElement>Flowchart.Svg(this.elementSvgG,"g", ["transform", "translate(0 50)"], ["operator-inputs"]);
        this.outputSvgG= <SVGGElement>Flowchart.Svg(this.elementSvgG,"g", ["transform", "translate(140 50)"], ["operator-outputs"]);
        this.debugInfoSvgText = <SVGTextElement>Flowchart.Svg(this.elementSvgG, "text", ["x", "0", "y", "100"],["operator-debuginfo"]);
        this.debugInfoSvgText.textContent="No debug info";

        this.box.onclick = (e) => {
            console.log("FlowchartOperator this.box.onclick");
            parent._notifyOperatorClicked(this, e);
        };
        
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
    get Caption() { return this.caption; }

    get InputsKVIt(){return this.Inputs.entries()}
    get OutputsKVIt(){return this.Outputs.entries()}
    public GetOutputConnectorByIndex=(i:number)=>this.Outputs[i];
    public GetInputConnectorByIndex=(i:number)=>this.Inputs[i];

    public RemoveFromDOM(): void {
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
        this.debugInfoSvgText.setAttribute("y", ""+height);
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

    public PopulateProperyGrid(parent:HTMLTableElement):boolean
    {
        //let tr=Flowchart.Html(parent, "tr", [],["develop-propertygrid-tr"]);
        //Flowchart.Html(tr, "td", [],["develop-propertygrid-td"], "AKey");
        //Flowchart.Html(tr, "td", [],["develop-propertygrid-td"], "AValue");
        return false;
    }

    protected serializeU32(ctx:SerializeContext, theNumber:number)
    {
        ctx.buffer.setUint32(ctx.bufferOffset, theNumber, true);
        ctx.bufferOffset+=4;
    }

    protected serializeS32(ctx:SerializeContext, theNumber:number)
    {
        ctx.buffer.setInt32(ctx.bufferOffset, theNumber, true);
        ctx.bufferOffset+=4;
    }
    
    protected SerializeInputsAndOutputs(ctx:SerializeContext)
    {
        for (const input of this.Inputs) {
            let variableAdress = 0;
            let links = input.GetLinksCopy();
            if(links.length==0){
                variableAdress=1; //because unconnected inputs read from adress 1 (which is "false", 0, 0.0, black...)
            }
            else{
                let out = links[0].From;
                variableAdress=ctx.typeIndex2globalConnectorIndex2adressOffset.get(out.Type)!.get(out.GlobalConnectorIndex)||1;
            }
            this.serializeU32(ctx, variableAdress);
        }
        for(const output of this.Outputs)
        {
            let variableAdress = 0;
            if(output.LinksLength==0){
                variableAdress=0; //because unconnected outputs write to adress 0 (which is never read!)
            }
            else{
                variableAdress=ctx.typeIndex2globalConnectorIndex2adressOffset.get(output.Type)!.get(output.GlobalConnectorIndex)||1;
            }
            this.serializeU32(ctx, variableAdress);
        }
    }

    public SerializeToBinary(ctx:SerializeContext)
    {
        //serialize Type
        this.serializeU32(ctx, this.TypeInfo.GlobalTypeIndex);
        //Index of instance
        this.serializeU32(ctx, this.GlobalOperatorIndex);
        this.SerializeInputsAndOutputs(ctx);
        this.SerializeFurtherProperties(ctx);
    }
    
    protected SerializeFurtherProperties(mapper:SerializeContext):void{
        return;
    }
}
