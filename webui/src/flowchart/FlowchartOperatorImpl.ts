import {FlowchartOperator, TypeInfo, PositionType, SingletonType} from "./FlowchartOperator";
import {Flowchart} from "./Flowchart";
import {FlowchartInputConnector, FlowchartOutputConnector, ConnectorType} from "./FlowchartConnector";
import { SerializeContextAndAdressMap } from "./FlowchartCompiler";
import {$, KeyValueTuple, StringNumberTuple} from "./../Utils";
import * as Song from "./Songs";
import { SimulationContext } from "./SimulationContext";

const Logic="Logic";
const Arithmetic="Arithmetic";
const Input="Input";
const Sensor = "Sensor";
const Output="Output";
const Timing="Timing";
const Converter="Converter";
const Sound = "Sound";
const Control = "Control";
const Custom ="Custom";

const CONSTANT = "Constant";
const SONG_INDEX = "Song Index"

export class OperatorRegistry{
    IsIndexKnown(globalTypeIndex: number) {
        return this.index2Info.has(globalTypeIndex);
    }
    
    private index2Info = new Map<number, TypeInfo>();
    private groupName2operatorName2Info = new Map<string, Map<string, TypeInfo>>();
    
    private Register(globalTypeIndex:number, groupName:string, operatorName:string, position:PositionType, singleton:SingletonType, builder:(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null)=>FlowchartOperator)
    {
        let ti:TypeInfo=new TypeInfo(globalTypeIndex, groupName, operatorName, position, singleton, builder)
        if(this.index2Info.has(globalTypeIndex)) throw new Error(`this.index2Info.has(globalTypeIndex) for ${groupName}::${operatorName}`);
        this.index2Info.set(globalTypeIndex, ti);
        if(!this.groupName2operatorName2Info.has(groupName)) this.groupName2operatorName2Info.set(groupName, new Map<string, TypeInfo>());
        let operatorName2Info = this.groupName2operatorName2Info.get(groupName)!;
        if(operatorName2Info.has(operatorName)) throw new Error(`operatorName2Info.has(operatorName) for ${groupName}::${operatorName}`);
        operatorName2Info.set(operatorName, ti);
    }

    public CreateByIndex(index:number, parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null):FlowchartOperator|null
    {
        let ti=this.index2Info.get(index);
        if(ti===undefined) return null;
        return ti.Builder(parent, caption, ti, configurationData);
    }

    public GetTypeInfo(index:number):TypeInfo|null
    {
        let ti=this.index2Info.get(index);
        if(ti===undefined) return null;
        return ti;
    }

    public populateOperatorLib(parent: HTMLDivElement, onmousedownHandler: (e:MouseEvent, ti:TypeInfo)=>any) { 
        let y = 10;
        let top = $.Html(parent, "ul", [], []);
        for (const kv of this.groupName2operatorName2Info.entries()) {
            let groupName = kv[0];
            $.Html(top, "li", [], [], groupName);
            let ul = $.Html(top, "ul", [], ["nested"]);
            for (const info of kv[1].values()) {
                let li = $.Html(ul, "li", [], [], info.OperatorName);
                li.onmousedown = (e) => onmousedownHandler(e, info);
            }
        }
    }

    public static Build():OperatorRegistry{
        let r:OperatorRegistry = new OperatorRegistry();
        r.Register(1, Logic, "AND", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Logic_ANDOperator(p, ca, ti, co));
        r.Register(2, Logic, "OR", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Logic_OROperator(p, ca, ti, co));
        r.Register(3, Arithmetic, "ADD", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Arithmetic_ADDOperator(p, ca, ti, co))
        r.Register(4, Arithmetic, "MULTIPLY", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Arithmetic_MULTIPLYOperator(p, ca, ti, co));
        r.Register(5, Arithmetic, "MAX", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Arithmetic_MAXOperator(p, ca, ti, co))
        r.Register(6, Arithmetic, "MIN", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Arithmetic_MINOperator(p, ca, ti, co))
        r.Register(7, Logic, "RS", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Logic_RSOperator(p, ca, ti, co));
        r.Register(8, Logic, "NOT", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Logic_NotOperator(p, ca, ti, co));
        r.Register(9, Input, "GreenButton", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Input_GreenButtonOperator(p, ca, ti, co));
        r.Register(10, Input, "EncoderButton", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Input_EncoderButtonOperator(p, ca, ti, co));
        r.Register(11, Input, "RedButton", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Input_RedButtonOperator(p, ca, ti, co));
        r.Register(12, Sensor, "Movement", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Sensor_MovementOperator(p, ca, ti, co));
        r.Register(13, Sensor, "AmbientTemperature", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Sensor_AmbientTemperatureOperator(p, ca, ti, co));
        r.Register(14, Output, "Relay", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Output_RelayOperator(p, ca, ti, co));
        r.Register(15, Output, "RedLed", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Output_RedLedOperator(p, ca, ti, co));
        r.Register(16, Output, "YellowLed", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Output_YellowLedOperator(p, ca, ti, co));
        r.Register(17, Output, "GreenLed", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Output_GreenLedOperator(p, ca, ti, co));
        r.Register(18, Logic,"ConstTRUE", PositionType.Input, SingletonType.Default, (p, ca, ti, co)=>new Logic_ConstTRUEOperator(p, ca, ti, co));
        r.Register(19, Arithmetic, "ConstINT", PositionType.Input, SingletonType.Default, (p, ca, ti, co)=>new Arithmetic_ConstINTOperator(p, ca, ti, co));
        r.Register(20, Timing,"TON", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Timing_TONOperator(p, ca, ti, co));
        r.Register(21, Timing,"TOF", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Timing_TOFOperator(p, ca, ti, co));
        r.Register(22, Arithmetic,"GreaterThan", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Arithmetic_GreaterThanOperator(p, ca, ti, co));
        r.Register(23, Arithmetic,"LessThan", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Arithmetic_LessThanOperator(p, ca, ti, co));
        r.Register(24, Sensor, "AmbientLight", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Sensor_AmbientLightOperator(p, ca, ti, co));
        r.Register(25, Sensor, "HeaterTemperature", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Sensor_HeaterTemperatureOperator(p, ca, ti, co));
        r.Register(26, Converter, "Bool2Color", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Bool2ColorConvert(p, ca, ti, co));
        r.Register(27, Output, "LED3", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Output_Led3Operator(p, ca, ti, co));
        r.Register(28, Output, "LED4", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Output_Led4Operator(p, ca, ti, co));
        r.Register(29, Output, "LED5", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Output_Led5Operator(p, ca, ti, co));
        r.Register(30, Output, "LED6", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Output_Led6Operator(p, ca, ti, co));
        r.Register(31, Output, "LED7", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Output_Led7Operator(p, ca, ti, co));
        r.Register(32, Sound, "Melody", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Sound_Melody(p, ca, ti, co));
        r.Register(33, Control, "PID", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Control_PID(p, ca, ti, co));
        r.Register(100, Custom, "XYZXYZBlock", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Custom_XYZBlock(p, ca, ti, co))
        return r;
    }
}



export class Logic_ANDOperator extends FlowchartOperator {
    private A:FlowchartInputConnector;
    private B:FlowchartInputConnector;
    private C:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.A = new FlowchartInputConnector(this, "A", 0, ConnectorType.BOOLEAN);
        this.B = new FlowchartInputConnector(this, "B", 1, ConnectorType.BOOLEAN);
        this.C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([this.A, this.B], [this.C]);
    }

    OnSimulationStep(ctx:SimulationContext){
        let A = ctx.GetBoolean(this.A);
        let B = ctx.GetBoolean(this.B);
        ctx.SetBoolean(this.C, A && B);
    }
}

export class Logic_OROperator extends FlowchartOperator {
    private A:FlowchartInputConnector;
    private B:FlowchartInputConnector;
    private C:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.A = new FlowchartInputConnector(this, "A", 0, ConnectorType.BOOLEAN);
        this.B = new FlowchartInputConnector(this, "B", 1, ConnectorType.BOOLEAN);
        this.C = new FlowchartOutputConnector(this, "C", 0,ConnectorType.BOOLEAN);
        this.AppendConnectors([this.A, this.B], [this.C]);
    }

    OnSimulationStep(ctx:SimulationContext){
        let A = ctx.GetBoolean(this.A);
        let B = ctx.GetBoolean(this.B);
        ctx.SetBoolean(this.C, A || B);
    }
}

export class Arithmetic_ADDOperator extends FlowchartOperator {
    private A:FlowchartInputConnector;
    private B:FlowchartInputConnector;
    private C:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        this.B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        this.C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.INTEGER);
        this.AppendConnectors([this.A, this.B], [this.C]);
    }

    OnSimulationStep(ctx:SimulationContext){
        let A = ctx.GetInteger(this.A);
        let B = ctx.GetInteger(this.B);
        ctx.SetInteger(this.C, A + B);
    }
}

export class Arithmetic_MULTIPLYOperator extends FlowchartOperator {
    private A:FlowchartInputConnector;
    private B:FlowchartInputConnector;
    private C:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        this.B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        this.C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.INTEGER);
        this.AppendConnectors([this.A, this.B], [this.C]);
    }

    OnSimulationStep(ctx:SimulationContext){
        let A = ctx.GetInteger(this.A);
        let B = ctx.GetInteger(this.B);
        ctx.SetInteger(this.C, A * B);
    }
}

export class Arithmetic_MAXOperator extends FlowchartOperator {
    private A:FlowchartInputConnector;
    private B:FlowchartInputConnector;
    private C:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        this.B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        this.C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.INTEGER);
        this.AppendConnectors([this.A, this.B], [this.C]);
    }

    OnSimulationStep(ctx:SimulationContext){
        let A = ctx.GetInteger(this.A);
        let B = ctx.GetInteger(this.B);
        ctx.SetInteger(this.C, Math.max(A,B));
    }
}

export class Arithmetic_MINOperator extends FlowchartOperator {
    private A:FlowchartInputConnector;
    private B:FlowchartInputConnector;
    private C:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        this.B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        this.C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.INTEGER);
        this.AppendConnectors([this.A, this.B], [this.C]);
    }

    OnSimulationStep(ctx:SimulationContext){
        let A = ctx.GetInteger(this.A);
        let B = ctx.GetInteger(this.B);
        ctx.SetInteger(this.C, Math.min(A,B));
    }
}

export class Logic_RSOperator extends FlowchartOperator {
    private R:FlowchartInputConnector;
    private S:FlowchartInputConnector;
    private C:FlowchartOutputConnector;
    private state:boolean=false;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.R = new FlowchartInputConnector(this, "R", 0, ConnectorType.BOOLEAN);
        this.S = new FlowchartInputConnector(this, "S", 1, ConnectorType.BOOLEAN);
        this.C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([this.R, this.S], [this.C]);
    }
    OnSimulationStart(ctx:SimulationContext){
        this.state=false;
    }

    OnSimulationStep(ctx:SimulationContext){
        if(ctx.GetBoolean(this.R)) this.state=false;
        else if(ctx.GetBoolean(this.S)) this.state = true;
        ctx.SetBoolean(this.C, this.state);
    }
}

export class Logic_NotOperator extends FlowchartOperator {
    private IN:FlowchartInputConnector;
    private OUT:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.IN = new FlowchartInputConnector(this, "IN", 0, ConnectorType.BOOLEAN);
        this.OUT = new FlowchartOutputConnector(this, "OUT", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([this.IN], [this.OUT]);
    }

    OnSimulationStep(ctx:SimulationContext){
        ctx.SetBoolean(this.OUT, !ctx.GetBoolean(this.IN));
    }

}
class Input_CommonButtonOperator extends FlowchartOperator {
    private state:boolean=false;
    private O:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.O = new FlowchartOutputConnector(this, "IsPressed", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([], [this.O]);
        this.ElementSvgG.onclick=(e)=>{
            console.log("Input_CommonButtonOperator this.ElementSvgG.onclick");
            parent._notifyOperatorClicked(this, e);
            this.state=!this.state;
        }
    }

    OnSimulationStart(ctx:SimulationContext){
        this.state=false;
    }

    OnSimulationStep(ctx:SimulationContext){
        this.box.classList.remove(this.state?"False":"True");
        this.box.classList.add(this.state?"True":"False");
        ctx.SetBoolean(this.O, this.state);
    }

    OnSimulationStop(ctx:SimulationContext){
        this.box.classList.remove("False", "True");
    }
}


export class Input_GreenButtonOperator extends Input_CommonButtonOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
    }
}

export class Input_EncoderButtonOperator extends Input_CommonButtonOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
    }
}

export class Input_RedButtonOperator extends Input_CommonButtonOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
    }
}

export class Sensor_MovementOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let O = new FlowchartOutputConnector(this, "Movement", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class Sensor_AmbientTemperatureOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let O = new FlowchartOutputConnector(this, "Temperatur", 0, ConnectorType.INTEGER);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class Output_RelayOperator extends FlowchartOperator {
    private I:FlowchartInputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.I=new FlowchartInputConnector(this, "Relay", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([this.I], []);
    }

    OnSimulationStep(ctx:SimulationContext){
        let state = ctx.GetBoolean(this.I);
        this.box.classList.remove(state?"False":"True");
        this.box.classList.add(state?"True":"False");
    }
}


class Output_CommonLedOperator extends FlowchartOperator {
    protected  I:FlowchartInputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null, protected colorOnTRUE:string, protected colorOnFALSE:string) {
        super(parent, caption, ti, configurationData);
        this.I = new FlowchartInputConnector(this, "LED", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([this.I], []);
    }

    OnSimulationStep(ctx:SimulationContext){
        let state = ctx.GetBoolean(this.I);
        this.box.style.fill=state?this.colorOnTRUE:this.colorOnFALSE;
    }

    OnSimulationStop(ctx:SimulationContext){
        this.box.style.removeProperty("fill");
    }
}


export class Output_RedLedOperator extends Output_CommonLedOperator {

    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData, "red", "grey");
    }
}

export class Output_YellowLedOperator extends Output_CommonLedOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData, "yellow", "grey");
    }
}

export class Output_GreenLedOperator extends Output_CommonLedOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData, "green", "grey");
    }
}

export class Output_CommonRGBLedOperator extends FlowchartOperator {
    protected LED:FlowchartInputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.LED = new FlowchartInputConnector(this, "LED", 0, ConnectorType.COLOR);
        this.AppendConnectors([this.LED], []);
    }

    OnSimulationStep(ctx:SimulationContext){
        this.box.style.fill=ctx.GetColor(this.LED);
    }

    OnSimulationStop(ctx:SimulationContext){
        this.box.style.removeProperty("fill");
    }
}

export class Output_Led3Operator extends Output_CommonRGBLedOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
    }
}

export class Output_Led4Operator extends Output_CommonRGBLedOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
    }
}

export class Output_Led5Operator extends Output_CommonRGBLedOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
    }
}

export class Output_Led6Operator extends Output_CommonRGBLedOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
    }
}

export class Output_Led7Operator extends Output_CommonRGBLedOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
    }
}

export class Logic_ConstTRUEOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let O = new FlowchartOutputConnector(this, "TRUE", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
    }
}

export class Sound_Melody extends FlowchartOperator {
  
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let A = new FlowchartInputConnector(this, "Trigger", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([A], []);
        this.cfg_setDefault(SONG_INDEX, 0);
    }

    private songIndexHTMLSelect:HTMLSelectElement|null=null;
    public PopulateProperyGrid(tbody:HTMLTableSectionElement):boolean
    {
        this.songIndexHTMLSelect=$.InputSelect(tbody, Song.default(), SONG_INDEX, this.configurationData);
        return true;
    }

    public SavePropertyGrid(tbody:HTMLTableSectionElement){
        if(this.songIndexHTMLSelect==null) return;
        this.cfg_setValue(SONG_INDEX, parseInt(this.songIndexHTMLSelect.value));
    }

    protected SerializeFurtherProperties(ctx:SerializeContextAndAdressMap):void{
        ctx.ctx.writeS32(this.cfg_getValue(SONG_INDEX, 0));
        return;
    }
}

export class Arithmetic_ConstINTOperator extends FlowchartOperator {
    private O:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.O = new FlowchartOutputConnector(this, "Out", 0, ConnectorType.INTEGER);
        this.AppendConnectors([], [this.O]);
        this.cfg_setDefault(CONSTANT, 0);
    }

    private constantHTMLInput:HTMLInputElement|null=null;
    public PopulateProperyGrid(tbody:HTMLTableSectionElement):boolean
    {
        this.constantHTMLInput=$.InputNumber(tbody, -32768, 32767, CONSTANT, this.configurationData);
        return true;
    }

    public SavePropertyGrid(tbody:HTMLTableSectionElement){
        if(this.constantHTMLInput==null) return;
        this.cfg_setValue(CONSTANT, this.constantHTMLInput.valueAsNumber);
    }

    protected SerializeFurtherProperties(ctx:SerializeContextAndAdressMap):void{
        ctx.ctx.writeS32(this.cfg_getValue(CONSTANT, 0));
        return;
    }

    public OnSimulationStart(ctx:SimulationContext){
        ctx.SetInteger(this.O, this.cfg_getValue(CONSTANT, 0));
    }

}

const COLOR_TRUE="Color for TRUE";
const COLOR_FALSE="Color for FALSE";

export class Bool2ColorConvert extends FlowchartOperator {
    private IN:FlowchartInputConnector;
    private OUT:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.IN = new FlowchartInputConnector(this, "IN", 0, ConnectorType.BOOLEAN);
        this.OUT = new FlowchartOutputConnector(this, "OUT", 0, ConnectorType.COLOR);
        this.AppendConnectors([this.IN], [this.OUT]);
    }

    private colorTRUEHTMLInput:HTMLInputElement|null=null;
    private colorFALSEHTMLInput:HTMLInputElement|null=null;
    public PopulateProperyGrid(tbody:HTMLTableSectionElement):boolean
    {
        this.colorTRUEHTMLInput=$.InputColor(tbody, COLOR_TRUE, this.configurationData);
        this.colorFALSEHTMLInput=$.InputColor(tbody, COLOR_FALSE, this.configurationData);
        return true;
    }

    public SavePropertyGrid(tbody:HTMLTableSectionElement){
        if(this.colorTRUEHTMLInput==null || this.colorFALSEHTMLInput==null) return;
        this.cfg_setValue(COLOR_TRUE, this.colorTRUEHTMLInput.value);
        this.cfg_setValue(COLOR_FALSE, this.colorFALSEHTMLInput.value);
    }

    protected SerializeFurtherProperties(ctx:SerializeContextAndAdressMap):void{
        let colorString = this.cfg_getValue(COLOR_TRUE, "#ff0000");
        let colorNum=$.ColorDomString2ColorNum(colorString);
        ctx.ctx.writeU32(colorNum);
        colorString = this.cfg_getValue(COLOR_FALSE, "#000000");
        colorNum=$.ColorDomString2ColorNum(colorString);
        ctx.ctx.writeU32(colorNum);
        return;
    }

    public OnSimulationStep(ctx:SimulationContext){
        let currentInputValue = ctx.GetBoolean(this.IN);
        let color =currentInputValue?this.cfg_getValue(COLOR_TRUE, "RED"):this.cfg_getValue(COLOR_FALSE, "GREY");
        ctx.SetColor(this.OUT, color);
    }
}

export class Timing_TONOperator extends FlowchartOperator {
    private inputTRIGGER:FlowchartInputConnector;
    private inputPresetTime_msecs:FlowchartInputConnector;
    private output:FlowchartOutputConnector;
    private outputElapsedTime_msecs:FlowchartOutputConnector;
    private lastInputValue:boolean=false;
    private inputPositiveEdge:number = Number.MAX_VALUE;
    
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.inputTRIGGER = new FlowchartInputConnector(this, "TRIGGER", 0, ConnectorType.BOOLEAN);
        this.inputPresetTime_msecs = new FlowchartInputConnector(this, "PT_MS", 1, ConnectorType.INTEGER);
        this.output = new FlowchartOutputConnector(this, "OUT", 0, ConnectorType.BOOLEAN);
        this.outputElapsedTime_msecs = new FlowchartOutputConnector(this, "ET_MS", 0, ConnectorType.INTEGER);
        this.AppendConnectors([this.inputTRIGGER, this.inputPresetTime_msecs], [this.output,this.outputElapsedTime_msecs]);
    }

    public OnSimulationStart(ctx:SimulationContext){
        this.inputPositiveEdge = Number.MAX_VALUE;
    }

    public OnSimulationStep(ctx:SimulationContext){
        let currentInputValue = ctx.GetBoolean(this.inputTRIGGER);
        let presetTime_msecs = ctx.GetInteger(this.inputPresetTime_msecs);
        let now = ctx.GetMillis();
        if(this.lastInputValue==false && currentInputValue==true){
            this.inputPositiveEdge=now;
        }
        else if(currentInputValue==false){
            this.inputPositiveEdge=Number.MAX_VALUE;
        }
        this.lastInputValue=currentInputValue;
        let elapsed = (now-this.inputPositiveEdge);
        ctx.SetBoolean(this.output, elapsed>=presetTime_msecs);
        ctx.SetInteger(this.outputElapsedTime_msecs, elapsed);
    }
}

export class Timing_TOFOperator extends FlowchartOperator {
  
    private inputTRIGGER:FlowchartInputConnector;
    private inputPresetTime_msecs:FlowchartInputConnector;
    private output:FlowchartOutputConnector;
    private outputElapsedTime_msecs:FlowchartOutputConnector;
    private lastInputValue:boolean=false;
    private inputNegativeEdge:number = 0;
    
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.inputTRIGGER = new FlowchartInputConnector(this, "TRIGGER", 0, ConnectorType.BOOLEAN);
        this.inputPresetTime_msecs = new FlowchartInputConnector(this, "PT_MS", 1, ConnectorType.INTEGER);
        this.output = new FlowchartOutputConnector(this, "OUT", 0, ConnectorType.BOOLEAN);
        this.outputElapsedTime_msecs = new FlowchartOutputConnector(this, "ET_MS", 0, ConnectorType.INTEGER);
        this.AppendConnectors([this.inputTRIGGER, this.inputPresetTime_msecs], [this.output,this.outputElapsedTime_msecs]);
    }

    public OnSimulationStart(ctx:SimulationContext){
        this.inputNegativeEdge = 0;
    }

    public OnSimulationStep(ctx:SimulationContext){
        let currentInputValue = ctx.GetBoolean(this.inputTRIGGER);
        let presetTime_msecs = ctx.GetInteger(this.inputPresetTime_msecs);
        let now = ctx.GetMillis();
        if(this.lastInputValue==true && currentInputValue==false){
            this.inputNegativeEdge=now;
        }
        else if(currentInputValue==false){
            this.inputNegativeEdge=0;
        }
        this.lastInputValue=currentInputValue;
        let elapsed = (now-this.inputNegativeEdge);
        elapsed=Math.min(elapsed, presetTime_msecs)
        ctx.SetBoolean(this.output, currentInputValue || (elapsed<presetTime_msecs));
        ctx.SetInteger(this.outputElapsedTime_msecs, elapsed);
    }
}

export class Arithmetic_GreaterThanOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([A, B], [C]);
    }
}

export class Arithmetic_LessThanOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([A, B], [C]);
    }
}

export class Sensor_AmbientLightOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let O = new FlowchartOutputConnector(this, "Lux", 0, ConnectorType.INTEGER);
        this.AppendConnectors([], [O]);
    }
}

export class Sensor_HeaterTemperatureOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let O = new FlowchartOutputConnector(this, "Degrees", 0, ConnectorType.INTEGER);
        this.AppendConnectors([], [O]);
    }
}

export class Custom_XYZBlock extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let A = new FlowchartInputConnector(this, "TempVL", 0, ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "TempRL", 1, ConnectorType.INTEGER);
        let B1 = new FlowchartInputConnector(this, "Switch", 2, ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "Motor", 0,ConnectorType.BOOLEAN);
        let C1 = new FlowchartOutputConnector(this, "Valve", 1,ConnectorType.INTEGER);
        this.AppendConnectors([A, B, B1], [C, C1]);
    }

    private value1HTMLInput:HTMLInputElement|null=null;
    private value2HTMLInput:HTMLInputElement|null=null;
    private value3HTMLInput:HTMLInputElement|null=null;
    private color1HTMLInput:HTMLInputElement|null=null;
    public PopulateProperyGrid(tbody:HTMLTableSectionElement):boolean
    {
        this.value1HTMLInput=$.InputNumber(tbody, -32768, 32767, "Value1", this.configurationData);
        this.value1HTMLInput=$.InputNumber(tbody, -32768, 32767, "Value2", this.configurationData);
        this.value1HTMLInput=$.InputNumber(tbody, -32768, 32767, "Value3", this.configurationData);
        this.color1HTMLInput=$.InputColor(tbody, "Color1", this.configurationData);
        return true;
    }

    public SavePropertyGrid(tbody:HTMLTableSectionElement){
        this.cfg_setValue("Value1", this.value1HTMLInput!.valueAsNumber);
        this.cfg_setValue("Value2", this.value2HTMLInput!.valueAsNumber);
        this.cfg_setValue("Value3", this.value3HTMLInput!.valueAsNumber);
        this.cfg_setValue("Color1", this.color1HTMLInput!.valueAsNumber);
    }

    protected SerializeFurtherProperties(ctx:SerializeContextAndAdressMap):void{
        ctx.ctx.writeS32(this.cfg_getValue("Value1", 0));
        ctx.ctx.writeS32(this.cfg_getValue("Value2", 0));
        ctx.ctx.writeS32(this.cfg_getValue("Value3", 0));
        let colorString = this.cfg_getValue("Color1", "#000000");
        let colorNum=$.ColorDomString2ColorNum(colorString);
        ctx.ctx.writeU32(colorNum);
        return;
    }
}

export class Control_PID extends FlowchartOperator {
    private inputActualValue:FlowchartInputConnector;
    private inputSetpoint:FlowchartInputConnector;
    private inputKP:FlowchartInputConnector;
    private inputKI:FlowchartInputConnector;
    private inputKD:FlowchartInputConnector;
    private output:FlowchartOutputConnector;


    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.inputActualValue = new FlowchartInputConnector(this, "Actual", 0, ConnectorType.FLOAT);
        this.inputSetpoint = new FlowchartInputConnector(this, "Setpoint", 1, ConnectorType.FLOAT);
        this.inputKP= new FlowchartInputConnector(this, "KP", 2, ConnectorType.FLOAT);
        this.inputKI= new FlowchartInputConnector(this, "KI", 3, ConnectorType.FLOAT);
        this.inputKD= new FlowchartInputConnector(this, "KD", 4, ConnectorType.FLOAT);
        this.output = new FlowchartOutputConnector(this, "Out", 1,ConnectorType.INTEGER);
        this.AppendConnectors([this.inputActualValue, this.inputSetpoint, this.inputKP, this.inputKI, this.inputKD], [this.output]);
    }
}