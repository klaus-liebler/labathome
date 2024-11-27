import {FlowchartInputConnector, FlowchartOutputConnector } from "./FlowchartConnector";
import {Flowchart} from "./Flowchart";
import { SerializeContextAndAdressMap } from "./FlowchartCompiler";
import {KeyValueTuple, Svg} from "../utils/common"
import { SimulationContext } from "./SimulationContext";

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
    constructor(
        public GlobalTypeIndex:number, 
        public GroupName:string, 
        public OperatorName:string, 
        public Position:PositionType, 
        public Singleton:SingletonType, 
        public Builder:(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null)=>FlowchartOperator)
        {}
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
    private lastMouseDownDt:number=0;;

    get TypeInfo(){return this.typeInfo;}

    get Xpos(){return this.x;}
    get Ypos(){return this.y;}
    get Config_Copy(){
        return this.configurationData?this.configurationData.slice(0):null;
    }

    private x=0;
    private y=0;

    protected box:SVGRectElement;

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

    protected cfg_setDefault(key:string, value:any)
    {
        if(this.configurationData==null) this.configurationData=[];
        for (const e of this.configurationData) {
            if(e.key==key){
                return;
            }
        } 
        this.configurationData.push({key:key, value:value});
    }

    protected cfg_getValue(key:string, defaultValue:any):any
    {
        if(this.configurationData==null) this.configurationData=[];
        for (const e of this.configurationData) {
            if(e.key==key){
                return e.value;
            }
        };
        this.configurationData.push({key:key, value:defaultValue});
        return defaultValue;
    }

    protected cfg_setValue(key:string, value:any)
    {
        if(this.configurationData==null) this.configurationData=[];
        for (const e of this.configurationData) {
            if(e.key==key){
                e.value=value;
                return;
            }
        } 
        this.configurationData.push({key:key, value:value});
    }

    constructor(private parent: Flowchart, private caption: string, private typeInfo: TypeInfo, protected configurationData:KeyValueTuple[]|null) {
        this.index = FlowchartOperator.MAX_INDEX++;
        this.elementSvgG = <SVGGElement>Svg(parent.OperatorsLayer, "g", [], ["operator"]);
        this.elementSvgG.setAttribute('data-operator-index', "" + this.index);
        let dragGroup = <SVGGElement>Svg(this.elementSvgG, "g", [], []);
        this.box = <SVGRectElement>Svg(dragGroup, "rect", ["width","140", "height", "100", "rx", "10", "ry", "10"], ["operator-box"]);
        let title = <SVGTextElement>Svg(dragGroup,"text", ["x", "5", "y", "21"],["operator-title"]);
        title.textContent = caption;
        this.debugInfoSvgText = <SVGTextElement>Svg(dragGroup, "text", ["x", "0", "y", "100"],["operator-debuginfo"]);
        this.debugInfoSvgText.textContent="No debug info";

        this.inputSvgG= <SVGGElement>Svg(this.elementSvgG,"g", ["transform", "translate(0 50)"], ["operator-inputs"]);
        this.outputSvgG= <SVGGElement>Svg(this.elementSvgG,"g", ["transform", "translate(140 50)"], ["operator-outputs"]);


        this.elementSvgG.onmousedown = (e) => {
            this.lastMouseDownDt=Date.now()
            //console.log(`FlowchartOperator ${this.Caption} onmousedown ${this.lastMouseDownDt}`);
        };
        this.elementSvgG.onmouseup = (e) => {
            var diff = Date.now()-this.lastMouseDownDt
            //console.log(`FlowchartOperator ${this.Caption} onmouseup diff=${diff}`);
            if(diff<400){
                parent._notifyOperatorClicked(this, e);
            }
        };
        
        if (this.parent.Options.canUserMoveOperators) {
            dragGroup.onmousedown = (e) => {
                this.RegisterDragging(e);
            }
        }
    }
    public RegisterDragging(e:MouseEvent)
    {
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

    public PopulateProperyGrid(parent:HTMLTableSectionElement):boolean
    {
        return false;
    }

    public SavePropertyGrid(tbody:HTMLTableSectionElement){
        return;
    }

    public OnSimulationStart(ctx:SimulationContext){
        return;
    }

    public OnSimulationStep(ctx:SimulationContext){
        return;
    }

    public OnSimulationStop(ctx:SimulationContext){
        return;
    }

    
    protected SerializeInputsAndOutputs(ctx:SerializeContextAndAdressMap)
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
            ctx.ctx.writeU32(variableAdress);
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
            ctx.ctx.writeU32(variableAdress);
        }
    }

    public SerializeToBinary(ctx:SerializeContextAndAdressMap)
    {
        //serialize Type
        ctx.ctx.writeU32(this.TypeInfo.GlobalTypeIndex);
        //Index of instance
        ctx.ctx.writeU32(this.GlobalOperatorIndex);
        this.SerializeInputsAndOutputs(ctx);
        this.SerializeFurtherProperties(ctx);
    }
    
    protected SerializeFurtherProperties(mapper:SerializeContextAndAdressMap):void{
        return;
    }
}
