import {FlowchartOperator, TypeInfo, PositionType, SingletonType} from "./FlowchartOperator";
import {Flowchart, KeyValueTuple} from "./Flowchart";
import {FlowchartInputConnector, FlowchartOutputConnector, ConnectorType} from "./FlowchartConnector";
import { SerializeContextAndAdressMap } from "./FlowchartCompiler";
import {$} from "./../Utils"

const Logic="Logic";
const Arithmetic="Arithmetic";
const Input="Input";
const Sensor = "Sensor";
const Output="Output";
const Timing="Timing";
const Converter="Converter";

const CONSTANT = "Constant";

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
        return r;
    }
}



export class Logic_ANDOperator extends FlowchartOperator {
  
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.BOOLEAN);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([A, B], [C]);
    }
}

export class Logic_OROperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.BOOLEAN);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", 0,ConnectorType.BOOLEAN);
        this.AppendConnectors([A, B], [C]);
    }
}

export class Arithmetic_ADDOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}

export class Arithmetic_MULTIPLYOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}

export class Arithmetic_MAXOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}

export class Arithmetic_MINOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}

export class Logic_RSOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let R = new FlowchartInputConnector(this, "R", 0, ConnectorType.BOOLEAN);
        let S = new FlowchartInputConnector(this, "S", 1, ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([R, S], [C]);
    }
}

export class Logic_NotOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([A], [C]);
    }

}

export class Input_GreenButtonOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let O = new FlowchartOutputConnector(this, "IsPressed", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class Input_EncoderButtonOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let O = new FlowchartOutputConnector(this, "IsPressed", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class Input_RedButtonOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let O = new FlowchartOutputConnector(this, "IsPressed", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
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
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let conn = new FlowchartInputConnector(this, "Relay", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class Output_RedLedOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let conn = new FlowchartInputConnector(this, "LED", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class Output_YellowLedOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let conn = new FlowchartInputConnector(this, "LED", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class Output_GreenLedOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let conn = new FlowchartInputConnector(this, "LED", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class Output_Led3Operator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let conn = new FlowchartInputConnector(this, "LED", 0, ConnectorType.COLOR);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class Output_Led4Operator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let conn = new FlowchartInputConnector(this, "LED", 0, ConnectorType.COLOR);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class Output_Led5Operator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let conn = new FlowchartInputConnector(this, "LED", 0, ConnectorType.COLOR);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class Output_Led6Operator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let conn = new FlowchartInputConnector(this, "LED", 0, ConnectorType.COLOR);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class Output_Led7Operator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let conn = new FlowchartInputConnector(this, "LED", 0, ConnectorType.COLOR);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class Logic_ConstTRUEOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let O = new FlowchartOutputConnector(this, "TRUE", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}


export class Arithmetic_ConstINTOperator extends FlowchartOperator {
    
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let O = new FlowchartOutputConnector(this, "Out", 0, ConnectorType.INTEGER);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
        this.cfg_setDefault(CONSTANT, 0);
    }

    private constantHTMLInput:HTMLInputElement|null=null;
    public PopulateProperyGrid(tbody:HTMLTableSectionElement):boolean
    {
        this.constantHTMLInput=PropertyGridHelpers.Number(tbody, -32768, 32767, CONSTANT, this.configurationData);
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
}

const COLOR_TRUE="Color for TRUE";
const COLOR_FALSE="Color for FALSE";

export class Bool2ColorConvert extends FlowchartOperator {
    
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let IN = new FlowchartInputConnector(this, "IN", 0, ConnectorType.BOOLEAN);
        let OUT = new FlowchartOutputConnector(this, "OUT", 0, ConnectorType.COLOR);
        this.AppendConnectors([IN], [OUT]);
    }

    private colorTRUEHTMLInput:HTMLInputElement|null=null;
    private colorFALSEHTMLInput:HTMLInputElement|null=null;
    public PopulateProperyGrid(tbody:HTMLTableSectionElement):boolean
    {
        this.colorTRUEHTMLInput=PropertyGridHelpers.Color(tbody, COLOR_TRUE, this.configurationData);
        this.colorFALSEHTMLInput=PropertyGridHelpers.Color(tbody, COLOR_FALSE, this.configurationData);
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
}

export class Timing_TONOperator extends FlowchartOperator {
  
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let A = new FlowchartInputConnector(this, "TRIGGER", 0, ConnectorType.BOOLEAN);
        let B = new FlowchartInputConnector(this, "PT_MS", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "OUT", 0, ConnectorType.BOOLEAN);
        let D = new FlowchartOutputConnector(this, "ET_MS", 0, ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C,D]);
    }
}

export class Timing_TOFOperator extends FlowchartOperator {
  
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let A = new FlowchartInputConnector(this, "TRIGGER", 0, ConnectorType.BOOLEAN);
        let B = new FlowchartInputConnector(this, "PresetTimeMS", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "OUT", 0, ConnectorType.BOOLEAN);
        let D = new FlowchartOutputConnector(this, "ElapsedTimeMS", 0, ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C,D]);
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

export class Custom_XYZOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let A = new FlowchartInputConnector(this, "TempVL", 0, ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "TempRL", 1, ConnectorType.INTEGER);
        let B1 = new FlowchartInputConnector(this, "Switch", 2, ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "Motor", 0,ConnectorType.BOOLEAN);
        let C1 = new FlowchartOutputConnector(this, "Stellventil", 1,ConnectorType.INTEGER);
        this.AppendConnectors([A, B, B1], [C, C1]);
    }
}

class PropertyGridHelpers
{
    public static Number(table:HTMLTableSectionElement, min:number, max:number, key:string, cfg:KeyValueTuple[]|null):HTMLInputElement
    {
        let value:number=0;
        if(cfg!=null)
        {
            for (const e of cfg) {
                if(e.key==key && !isNaN(e.value)){
                    value=e.value;
                    break;
                }
            }
        }
        let tr=$.Html(table, "tr", [],["develop-propertygrid-tr"]);
        $.Html(tr, "td", [],["develop-propertygrid-td"], key);
        let inputContainer = $.Html(tr, "td", [],["develop-propertygrid-td"]);
        return <HTMLInputElement>$.Html(inputContainer, "input", ["type", "number", "min", ""+Math.round(min), "max", ""+Math.round(max), "value", ""+Math.round(value),]);
    }

   

    public static Color(table:HTMLTableSectionElement, key:string, cfg:KeyValueTuple[]|null):HTMLInputElement
    {
        let value:string="#ff0000";
        if(cfg!=null)
        {
            for (const e of cfg) {
                if(e.key==key){
                    value=e.value;
                    break;
                }
            }
        }
        let tr=$.Html(table, "tr", [],["develop-propertygrid-tr"]);
        $.Html(tr, "td", [],["develop-propertygrid-td"], key);
        let inputContainer = $.Html(tr, "td", [],["develop-propertygrid-td"]);
        return <HTMLInputElement>$.Html(inputContainer, "input", ["type", "color",  "value", value]);
    }
}
