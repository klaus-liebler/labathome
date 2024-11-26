import {FlowchartOperator, TypeInfo, PositionType, SingletonType} from "./FlowchartOperator";
import {Flowchart} from "./Flowchart";
import {FlowchartInputConnector, FlowchartOutputConnector, ConnectorType} from "./FlowchartConnector";
import { SerializeContextAndAdressMap } from "./FlowchartCompiler";
import {ColorDomString2ColorNum, Html, InputColor, InputFloatNumber, InputIntegerNumber, InputSelect, KeyValueTuple, StringNumberTuple} from "../utils/common";
import * as Song from "./Songs";
import { SimulationContext } from "./SimulationContext";

const Basic="Basic";
const Arithmetic="Arithmetic";
const Input="Input";
const Sensor = "Sensor";
const Output="Output";
const Converter="Converter";
const Sound = "Sound";
const Control = "Control";
const Custom ="Custom";

const CONSTANT = "Constant";
const SONG_INDEX = "Song Index";


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
        let top = Html(parent, "ul", [], []);
        for (const kv of this.groupName2operatorName2Info.entries()) {
            let groupName = kv[0];
            Html(top, "li", [], [], groupName);
            let ul = Html(top, "ul", [], ["nested"]);
            for (const info of kv[1].values()) {
                let li = Html(ul, "li", [], [], info.OperatorName);
                li.onmousedown = (e) => onmousedownHandler(e, info);
            }
        }
    }

    public static Build():OperatorRegistry{
        let r:OperatorRegistry = new OperatorRegistry();
        r.Register(1, Basic, "AND", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Basic_ANDOperator(p, ca, ti, co));
        r.Register(2, Basic, "OR", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Basic_OROperator(p, ca, ti, co));
        r.Register(3, Basic, "XOR", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Basic_XOROperator(p, ca, ti, co));
        r.Register(4, Basic, "NOT", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Basic_NotOperator(p, ca, ti, co));
        r.Register(5, Basic, "RS", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Basic_RSOperator(p, ca, ti, co));
        r.Register(6, Basic, "SR", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Basic_SROperator(p, ca, ti, co));
        r.Register(7, Basic,"ConstTRUE", PositionType.Input, SingletonType.Default, (p, ca, ti, co)=>new Basic_ConstTRUEOperator(p, ca, ti, co));
        r.Register(8, Basic,"ConstFALSE", PositionType.Input, SingletonType.Default, (p, ca, ti, co)=>new Basic_ConstFALSEOperator(p, ca, ti, co));
        r.Register(9, Basic, "CNT", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Basic_CNTOperator(p, ca, ti, co));
        r.Register(10, Basic, "Timekeeper", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Basic_TimekeeperOperator(p, ca, ti, co));
        r.Register(11, Basic,"TON", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Basic_TONOperator(p, ca, ti, co));
        r.Register(12, Basic,"TOF", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Basic_TOFOperator(p, ca, ti, co));
        
        r.Register(13, Arithmetic, "ADD", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Arithmetic_ADDOperator(p, ca, ti, co));
        r.Register(14, Arithmetic, "SUB", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Arithmetic_SUBOperator(p, ca, ti, co));
        r.Register(15, Arithmetic, "MULTIPLY", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Arithmetic_MULTIPLYOperator(p, ca, ti, co));
        r.Register(16, Arithmetic, "DIVIDE", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Arithmetic_DIVIDEOperator(p, ca, ti, co));
        r.Register(17, Arithmetic, "MAX", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Arithmetic_MAXOperator(p, ca, ti, co));
        r.Register(18, Arithmetic, "MIN", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Arithmetic_MINOperator(p, ca, ti, co));
        r.Register(19, Arithmetic,"GreaterThan", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Arithmetic_GreaterThanOperator(p, ca, ti, co));
        r.Register(20, Arithmetic,"LessThan", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Arithmetic_LessThanOperator(p, ca, ti, co));
        r.Register(21, Arithmetic, "ConstFLOAT", PositionType.Input, SingletonType.Default, (p, ca, ti, co)=>new Arithmetic_ConstFLOATOperator(p, ca, ti, co));
        r.Register(22, Arithmetic, "ConstINTEGER", PositionType.Input, SingletonType.Default, (p, ca, ti, co)=>new Arithmetic_ConstINTEGEROperator(p, ca, ti, co));
        
        r.Register(23, Arithmetic, "LIMIT", PositionType.Input, SingletonType.Default, (p, ca, ti, co)=>new Arithmetic_LIMITOperator(p, ca, ti, co));
        r.Register(24, Arithmetic, "LIMITMONITOR", PositionType.Input, SingletonType.Default, (p, ca, ti, co)=>new Arithmetic_LIMITMONITOROperator(p, ca, ti, co));

        r.Register(25, Converter, "Bool2Color", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Bool2ColorConvert(p, ca, ti, co));
        r.Register(26, Converter, "Bool2Int", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Bool2IntConvert(p, ca, ti, co));
        r.Register(27, Converter, "Bool2Float", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Bool2FloatConvert(p, ca, ti, co));
        r.Register(28, Converter, "Int2Bool", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Int2BoolConvert(p, ca, ti, co));
        r.Register(29, Converter, "Int2Float", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Int2FloatConvert(p, ca, ti, co));
        r.Register(30, Converter, "Float2Int", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Float2IntConvert(p, ca, ti, co));
        
        r.Register(31, Input, "GreenButton", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Input_GreenButtonOperator(p, ca, ti, co));
        r.Register(32, Input, "EncoderButton", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Input_EncoderButtonOperator(p, ca, ti, co));
        r.Register(33, Input, "EncoderDetents", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Input_EncoderDetentsOperator(p, ca, ti, co));
        r.Register(34, Input, "RedButton", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Input_RedButtonOperator(p, ca, ti, co));
        r.Register(35, Input, "AnalogInput0", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Input_AnalogInput0Operator(p, ca, ti, co));
        r.Register(36, Input, "AnalogInput1", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Input_AnalogInput1Operator(p, ca, ti, co));
        r.Register(37, Input, "AnalogInput2", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Input_AnalogInput2Operator(p, ca, ti, co));
        r.Register(38, Input, "AnalogInput3", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Input_AnalogInput3Operator(p, ca, ti, co));
        
        r.Register(39, Sensor, "Movement", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Sensor_MovementOperator(p, ca, ti, co));
        r.Register(40, Sensor, "AirTemperature", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Sensor_AirTemperatureOperator(p, ca, ti, co));
        r.Register(41, Sensor, "AirHumidity", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Sensor_AirHumidityOperator(p, ca, ti, co));
        r.Register(42, Sensor, "AirPressure", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Sensor_AirPressureOperator(p, ca, ti, co));
        r.Register(43, Sensor, "AirCO2", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Sensor_AirCO2Operator(p, ca, ti, co));
        r.Register(44, Sensor, "AirQuality", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Sensor_AirQualityOperator(p, ca, ti, co));
        r.Register(45, Sensor, "AmbientBrightness", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Sensor_AmbientBrightnessOperator(p, ca, ti, co));
        r.Register(46, Sensor, "HeaterTemperature", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Sensor_HeaterTemperatureOperator(p, ca, ti, co));
        //r.Register(47, Sensor, "AmbientNoise", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Sensor_AmbientNoise(p, ca, ti, co));
        //r.Register(48, Sensor, "ExternalPressure", PositionType.Input, SingletonType.Singleton, (p, ca, ti, co)=>new Sensor_ExternalPressure(p, ca, ti, co));
        
        r.Register(49, Output, "Relay", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Output_RelayOperator(p, ca, ti, co));
        r.Register(50, Output, "RedLed", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Output_RedLedOperator(p, ca, ti, co));
        r.Register(51, Output, "YellowLed", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Output_YellowLedOperator(p, ca, ti, co));
        r.Register(52, Output, "GreenLed", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Output_GreenLedOperator(p, ca, ti, co));
        r.Register(53, Output, "LED3", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Output_Led3Operator(p, ca, ti, co));
        r.Register(54, Output, "Fan", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Output_FanOperator(p, ca, ti, co));
        //r.Register(55, Output, "Fan2", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Output_Fan2Operator(p, ca, ti, co));
        r.Register(56, Output, "PowerLed", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Output_PowerLedOperator(p, ca, ti, co));
        r.Register(57, Output, "AnalogOutput0", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Output_AnalogOutput0Operator(p, ca, ti, co));

        r.Register(58, Sound, "Sound", PositionType.Output, SingletonType.Singleton, (p, ca, ti, co)=>new Sound_Sound(p, ca, ti, co));
        
        r.Register(59, Control, "PID", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Control_PID(p, ca, ti, co));
        
        
        //r.Register(100, Custom, "XYZXYZBlock", PositionType.Default, SingletonType.Default, (p, ca, ti, co)=>new Custom_XYZBlock(p, ca, ti, co))
        return r;
    }
}

class Sensor_CommonSensorOperator extends FlowchartOperator {
    private sensorValue:number=0;
    private O:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null, nameOfOutput:string, readonly minOutput:number, readonly maxOutput:number) {
        super(parent, caption, ti, configurationData);
        this.O = new FlowchartOutputConnector(this, nameOfOutput, 0, ConnectorType.FLOAT);
        this.AppendConnectors([], [this.O]);
        this.ElementSvgG.onclick=(e)=>{
            console.log("Input_CommonButtonOperator this.ElementSvgG.onclick");
            parent._notifyOperatorClicked(this, e);
            this.sensorValue=this.sensorValue==this.minOutput?this.maxOutput:this.minOutput;
        }
    }
    OnSimulationStart(ctx:SimulationContext){
        this.sensorValue=this.minOutput;
    }
    OnSimulationStep(ctx:SimulationContext){
        ctx.SetInteger(this.O, this.sensorValue);
    }
}



export class Sensor_HeaterTemperatureOperator extends Sensor_CommonSensorOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData, "DegreesCelsius", 25, 65);
    }
}

export class Sensor_AirTemperatureOperator extends Sensor_CommonSensorOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData, "Temperature", 180, 250);
    }
}

export class Sensor_AirHumidityOperator extends Sensor_CommonSensorOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData, "RelHumid%", 40, 60);
    }
}
export class Sensor_AirPressureOperator extends Sensor_CommonSensorOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData, "Pa", 800, 1200);
    }
}

export class Sensor_AirCO2Operator extends Sensor_CommonSensorOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData, "CO2ppm", 400, 1500);
    }
}

export class Sensor_AirQualityOperator extends Sensor_CommonSensorOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData, "Quality%", 20, 80);
    }
}


export class Sensor_AmbientBrightnessOperator extends Sensor_CommonSensorOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData, "Lux", 200, 700);
    }
}

export class Sensor_AmbientNoise extends Sensor_CommonSensorOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData, "NoisedBA", 30, 80);
    }
}

export class Sensor_ExternalPressure extends Sensor_CommonSensorOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData, "Pa", 1000, 2000);
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

export class Basic_ANDOperator extends FlowchartOperator {
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

export class Basic_OROperator extends FlowchartOperator {
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

export class Basic_XOROperator extends FlowchartOperator {
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
        ctx.SetBoolean(this.C, A ? !B : B);
    }
}

export class Arithmetic_ADDOperator extends FlowchartOperator {
    private A:FlowchartInputConnector;
    private B:FlowchartInputConnector;
    private C:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.A = new FlowchartInputConnector(this, "A", 0, ConnectorType.FLOAT);
        this.B = new FlowchartInputConnector(this, "B", 1, ConnectorType.FLOAT);
        this.C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.FLOAT);
        this.AppendConnectors([this.A, this.B], [this.C]);
    }

    OnSimulationStep(ctx:SimulationContext){
        let A = ctx.GetFloat(this.A);
        let B = ctx.GetFloat(this.B);
        ctx.SetInteger(this.C, A + B);
    }
}

export class Arithmetic_SUBOperator extends FlowchartOperator {
    private A:FlowchartInputConnector;
    private B:FlowchartInputConnector;
    private C:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.A = new FlowchartInputConnector(this, "A", 0, ConnectorType.FLOAT);
        this.B = new FlowchartInputConnector(this, "B", 1, ConnectorType.FLOAT);
        this.C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.FLOAT);
        this.AppendConnectors([this.A, this.B], [this.C]);
    }

    OnSimulationStep(ctx:SimulationContext){
        let A = ctx.GetFloat(this.A);
        let B = ctx.GetFloat(this.B);
        ctx.SetFloat(this.C, A - B);
    }
}


export class Arithmetic_MULTIPLYOperator extends FlowchartOperator {
    private A:FlowchartInputConnector;
    private B:FlowchartInputConnector;
    private C:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.A = new FlowchartInputConnector(this, "A", 0, ConnectorType.FLOAT);
        this.B = new FlowchartInputConnector(this, "B", 1, ConnectorType.FLOAT);
        this.C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.FLOAT);
        this.AppendConnectors([this.A, this.B], [this.C]);
    }

    OnSimulationStep(ctx:SimulationContext){
        let A = ctx.GetFloat(this.A);
        let B = ctx.GetFloat(this.B);
        ctx.SetFloat(this.C, A * B);
    }
}

export class Arithmetic_DIVIDEOperator extends FlowchartOperator {
    private A:FlowchartInputConnector;
    private B:FlowchartInputConnector;
    private C:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.A = new FlowchartInputConnector(this, "A", 0, ConnectorType.FLOAT);
        this.B = new FlowchartInputConnector(this, "B", 1, ConnectorType.FLOAT);
        this.C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.FLOAT);
        this.AppendConnectors([this.A, this.B], [this.C]);
    }

    OnSimulationStep(ctx:SimulationContext){
        let A = ctx.GetFloat(this.A);
        let B = ctx.GetFloat(this.B);
        ctx.SetFloat(this.C, A / B);
    }
}

export class Arithmetic_MAXOperator extends FlowchartOperator {
    private A:FlowchartInputConnector;
    private B:FlowchartInputConnector;
    private C:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.A = new FlowchartInputConnector(this, "A", 0, ConnectorType.FLOAT);
        this.B = new FlowchartInputConnector(this, "B", 1, ConnectorType.FLOAT);
        this.C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.FLOAT);
        this.AppendConnectors([this.A, this.B], [this.C]);
    }

    OnSimulationStep(ctx:SimulationContext){
        let A = ctx.GetFloat(this.A);
        let B = ctx.GetFloat(this.B);
        ctx.SetFloat(this.C, Math.max(A,B));
    }
}

export class Arithmetic_MINOperator extends FlowchartOperator {
    private A:FlowchartInputConnector;
    private B:FlowchartInputConnector;
    private C:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.A = new FlowchartInputConnector(this, "A", 0, ConnectorType.FLOAT);
        this.B = new FlowchartInputConnector(this, "B", 1, ConnectorType.FLOAT);
        this.C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.FLOAT);
        this.AppendConnectors([this.A, this.B], [this.C]);
    }

    OnSimulationStep(ctx:SimulationContext){
        let A = ctx.GetFloat(this.A);
        let B = ctx.GetFloat(this.B);
        ctx.SetFloat(this.C, Math.min(A,B));
    }
}

export class Basic_RSOperator extends FlowchartOperator {
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

export class Basic_SROperator extends FlowchartOperator {
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
        if(ctx.GetBoolean(this.S)) this.state = true;
        else if(ctx.GetBoolean(this.R)) this.state=false;
        ctx.SetBoolean(this.C, this.state);
    }
}

export class Basic_CNTOperator extends FlowchartOperator {
    private CountUp:FlowchartInputConnector;
    private Reset:FlowchartInputConnector;
    private PresetValue:FlowchartInputConnector;
    private OUT:FlowchartOutputConnector;
    private CurrentValue:FlowchartOutputConnector;
    private _CurrentValue:number=0;
    private lastInputValue:boolean=false;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.CountUp = new FlowchartInputConnector(this, "CU", 0, ConnectorType.BOOLEAN);
        this.Reset = new FlowchartInputConnector(this, "Reset", 1, ConnectorType.BOOLEAN);
        this.PresetValue = new FlowchartInputConnector(this, "PV", 2, ConnectorType.INTEGER);
        this.OUT = new FlowchartOutputConnector(this, "OUT", 0, ConnectorType.BOOLEAN);
        this.CurrentValue = new FlowchartOutputConnector(this, "CV", 1, ConnectorType.BOOLEAN);
        this.AppendConnectors([this.CountUp, this.Reset, this.PresetValue], [this.OUT, this.CurrentValue]);
    }
    OnSimulationStart(ctx:SimulationContext){
        this._CurrentValue=0;
    }

    OnSimulationStep(ctx:SimulationContext){
        let currentInputValue = ctx.GetBoolean(this.CountUp);
        let PV=ctx.GetInteger(this.PresetValue);
        if(ctx.GetBoolean(this.Reset)){
            this._CurrentValue=0;
        }else if(this.lastInputValue==false && currentInputValue==true && this._CurrentValue<PV){
            this._CurrentValue++;
            console.log("Logic_CNTOperator this._CurrentValue++; "+this._CurrentValue);
        }
        ctx.SetBoolean(this.OUT, this._CurrentValue>=PV);//kann auch durch Veränderung des PV passieren, deshalb nicht im if
        ctx.SetInteger(this.CurrentValue, this._CurrentValue);
        this.lastInputValue=currentInputValue
    }
}


export class Basic_TimekeeperOperator extends FlowchartOperator {
    private CountUp:FlowchartInputConnector;
    private Reset:FlowchartInputConnector;
    private PresetValue:FlowchartInputConnector;
    private OUT:FlowchartOutputConnector;
    private CurrentValue:FlowchartOutputConnector;
    private _CurrentValueMs:number=0;
    private lastInputValue:boolean=false;
    private lastMillis:number=0;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.CountUp = new FlowchartInputConnector(this, "CU", 0, ConnectorType.BOOLEAN);
        this.Reset = new FlowchartInputConnector(this, "Reset", 1, ConnectorType.BOOLEAN);
        this.PresetValue = new FlowchartInputConnector(this, "PV_ms", 2, ConnectorType.INTEGER);
        this.OUT = new FlowchartOutputConnector(this, "OUT", 0, ConnectorType.BOOLEAN);
        this.CurrentValue = new FlowchartOutputConnector(this, "CV_ms", 1, ConnectorType.BOOLEAN);
        this.AppendConnectors([this.CountUp, this.Reset, this.PresetValue], [this.OUT, this.CurrentValue]);
    }
    OnSimulationStart(ctx:SimulationContext){
        this._CurrentValueMs=0;
    }

    OnSimulationStep(ctx:SimulationContext){
        let currentInputValue = ctx.GetBoolean(this.CountUp);
        let PV=ctx.GetInteger(this.PresetValue);
        if(ctx.GetBoolean(this.Reset)){
            this._CurrentValueMs=0;
            this.lastMillis=ctx.GetMillis();
        }else if(this.lastInputValue==false && currentInputValue==true && this._CurrentValueMs<PV){
            let now = ctx.GetMillis();//TODO: Fehler: Die Zeit wird nicht ständig aktualisiert - das kann nicht klappen!!!
            this._CurrentValueMs+=now-this.lastMillis;
            this.lastMillis=now;
            console.log("Logic_CNTOperator this._CurrentValue++; "+this._CurrentValueMs);
        }
        ctx.SetBoolean(this.OUT, this._CurrentValueMs>=PV);//kann auch durch Veränderung des PV passieren, deshalb nicht im if
        ctx.SetInteger(this.CurrentValue, this._CurrentValueMs);
        this.lastInputValue=currentInputValue
    }
}

export class Basic_NotOperator extends FlowchartOperator {
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

class Input_CommonAnalogInputOperator extends FlowchartOperator {
    private O:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.O = new FlowchartOutputConnector(this, "Voltage", 0, ConnectorType.FLOAT);
        this.AppendConnectors([], [this.O]);
    }

    OnSimulationStep(ctx:SimulationContext){
        ctx.SetFloat(this.O, 0);
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

export class Input_AnalogInput0Operator extends Input_CommonAnalogInputOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
    }
}

export class Input_AnalogInput1Operator extends Input_CommonAnalogInputOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
    }
}

export class Input_AnalogInput2Operator extends Input_CommonAnalogInputOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
    }
}

export class Input_AnalogInput3Operator extends Input_CommonAnalogInputOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
    }
}


export class Input_EncoderDetentsOperator extends FlowchartOperator {
    protected O:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.O = new FlowchartOutputConnector(this, "Detents", 0, ConnectorType.INTEGER);
        this.AppendConnectors([], [this.O]);
    }

    public OnSimulationStart(ctx: SimulationContext): void {
        ctx.SetInteger(this.O, 20);
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

export class Output_FanOperator extends FlowchartOperator {
    private I:FlowchartInputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.I=new FlowchartInputConnector(this, "Power%", 0, ConnectorType.FLOAT);
        this.AppendConnectors([this.I], []);
    }

    OnSimulationStep(ctx:SimulationContext){
        let state = ctx.GetInteger(this.I);
        this.box.innerHTML=state+"%";
    }
}



export class Output_AnalogOutput0Operator extends FlowchartOperator {
    private I:FlowchartInputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.I=new FlowchartInputConnector(this, "OutVolt", 0, ConnectorType.FLOAT);
        this.AppendConnectors([this.I], []);
    }

    OnSimulationStep(ctx:SimulationContext){
        let state = ctx.GetFloat(this.I);
        this.box.innerHTML=state+"Volt";
    }
}

export class Output_PowerLedOperator extends FlowchartOperator {
    private I:FlowchartInputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.I=new FlowchartInputConnector(this, "Power%", 0, ConnectorType.FLOAT);
        this.AppendConnectors([this.I], []);
    }

    OnSimulationStep(ctx:SimulationContext){
        let state = ctx.GetFloat(this.I);
        this.box.innerHTML=state+"%";
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


export class Basic_ConstTRUEOperator extends FlowchartOperator {
    protected O:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.O = new FlowchartOutputConnector(this, "TRUE", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([], [this.O]);
    }

    OnSimulationStart(ctx:SimulationContext){
        ctx.SetBoolean(this.O, false);
    }
}

export class Basic_ConstFALSEOperator extends FlowchartOperator {

    protected O:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.O = new FlowchartOutputConnector(this, "TRUE", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([], [this.O]);
    }

    OnSimulationStart(ctx:SimulationContext){
        ctx.SetBoolean(this.O, true);
    }
}

export class Sound_Sound extends FlowchartOperator {
  
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let A = new FlowchartInputConnector(this, "Trigger", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([A], []);
        this.cfg_setDefault(SONG_INDEX, 0);
    }

    private songIndexHTMLSelect:HTMLSelectElement|null=null;
    public PopulateProperyGrid(tbody:HTMLTableSectionElement):boolean
    {
        this.songIndexHTMLSelect=InputSelect(tbody, Song.default(), SONG_INDEX, this.configurationData);
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

export class Arithmetic_ConstFLOATOperator extends FlowchartOperator {
    private O:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.O = new FlowchartOutputConnector(this, "Out", 0, ConnectorType.FLOAT);
        this.AppendConnectors([], [this.O]);
        this.cfg_setDefault(CONSTANT, 0);
    }

    private constantHTMLInput:HTMLInputElement|null=null;
    public PopulateProperyGrid(tbody:HTMLTableSectionElement):boolean
    {
        this.constantHTMLInput=InputFloatNumber(tbody, CONSTANT, this.configurationData);
        return true;
    }

    public SavePropertyGrid(tbody:HTMLTableSectionElement){
        if(this.constantHTMLInput==null) return;
        this.cfg_setValue(CONSTANT, this.constantHTMLInput.valueAsNumber);
    }

    protected SerializeFurtherProperties(ctx:SerializeContextAndAdressMap):void{
        ctx.ctx.writeF32(this.cfg_getValue(CONSTANT, 0));
        return;
    }

    public OnSimulationStart(ctx:SimulationContext){
        ctx.SetFloat(this.O, this.cfg_getValue(CONSTANT, 0));
    }
}

export class Arithmetic_ConstINTEGEROperator extends FlowchartOperator {
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
        this.constantHTMLInput=InputIntegerNumber(tbody, -2000000000, +2000000000, CONSTANT, this.configurationData);
        return true;
    }

    public SavePropertyGrid(tbody:HTMLTableSectionElement){
        if(this.constantHTMLInput==null) return;
        this.cfg_setValue(CONSTANT, this.constantHTMLInput.valueAsNumber);
    }

    protected SerializeFurtherProperties(ctx:SerializeContextAndAdressMap):void{
        ctx.ctx.writeF32(this.cfg_getValue(CONSTANT, 0));
        return;
    }

    public OnSimulationStart(ctx:SimulationContext){
        ctx.SetFloat(this.O, this.cfg_getValue(CONSTANT, 0));
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
        this.colorTRUEHTMLInput=InputColor(tbody, COLOR_TRUE, this.configurationData);
        this.colorFALSEHTMLInput=InputColor(tbody, COLOR_FALSE, this.configurationData);
        return true;
    }

    public SavePropertyGrid(tbody:HTMLTableSectionElement){
        if(this.colorTRUEHTMLInput==null || this.colorFALSEHTMLInput==null) return;
        this.cfg_setValue(COLOR_TRUE, this.colorTRUEHTMLInput.value);
        this.cfg_setValue(COLOR_FALSE, this.colorFALSEHTMLInput.value);
    }

    protected SerializeFurtherProperties(ctx:SerializeContextAndAdressMap):void{
        let colorString = this.cfg_getValue(COLOR_TRUE, "#ff0000");
        let colorNum=ColorDomString2ColorNum(colorString);
        ctx.ctx.writeU32(colorNum);
        colorString = this.cfg_getValue(COLOR_FALSE, "#000000");
        colorNum=ColorDomString2ColorNum(colorString);
        ctx.ctx.writeU32(colorNum);
        return;
    }

    public OnSimulationStep(ctx:SimulationContext){
        let currentInputValue = ctx.GetBoolean(this.IN);
        let color =currentInputValue?this.cfg_getValue(COLOR_TRUE, "RED"):this.cfg_getValue(COLOR_FALSE, "GREY");
        ctx.SetColor(this.OUT, color);
    }
}

const NUMBER_TRUE="Number for TRUE";
const NUMBER_FALSE="Number for FALSE";

export class Bool2IntConvert extends FlowchartOperator {
    private IN:FlowchartInputConnector;
    private OUT:FlowchartOutputConnector;
    private numberTRUEHTMLInput:HTMLInputElement|null=null;
    private numberFALSEHTMLInput:HTMLInputElement|null=null;
    
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.IN = new FlowchartInputConnector(this, "IN", 0, ConnectorType.BOOLEAN);
        this.OUT = new FlowchartOutputConnector(this, "OUT", 0, ConnectorType.INTEGER);
        this.AppendConnectors([this.IN], [this.OUT]);
    }

    public PopulateProperyGrid(tbody:HTMLTableSectionElement):boolean
    {
        this.numberTRUEHTMLInput=InputIntegerNumber(tbody, Number.MIN_VALUE, Number.MAX_VALUE, NUMBER_TRUE, this.configurationData);
        this.numberFALSEHTMLInput=InputIntegerNumber(tbody, Number.MIN_VALUE, Number.MAX_VALUE, NUMBER_FALSE, this.configurationData);
        return true;
    }

    public SavePropertyGrid(tbody:HTMLTableSectionElement){
        if(this.numberFALSEHTMLInput==null || this.numberTRUEHTMLInput==null) return;
        this.cfg_setValue(NUMBER_TRUE, this.numberTRUEHTMLInput.valueAsNumber);
        this.cfg_setValue(NUMBER_FALSE, this.numberFALSEHTMLInput.valueAsNumber);
    }

    protected SerializeFurtherProperties(ctx:SerializeContextAndAdressMap):void{
        ctx.ctx.writeS32(this.cfg_getValue(NUMBER_TRUE, 1));
        ctx.ctx.writeS32(this.cfg_getValue(NUMBER_FALSE, 0));
        return;
    }

    public OnSimulationStep(ctx:SimulationContext){
        let i = ctx.GetBoolean(this.IN);
        ctx.SetInteger(this.OUT, i?this.cfg_getValue(NUMBER_TRUE, 1):this.cfg_getValue(NUMBER_FALSE, 0));
    }
}

export class Bool2FloatConvert extends FlowchartOperator {
    private IN:FlowchartInputConnector;
    private OUT:FlowchartOutputConnector;
    private numberTRUEHTMLInput:HTMLInputElement|null=null;
    private numberFALSEHTMLInput:HTMLInputElement|null=null;
    
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.IN = new FlowchartInputConnector(this, "IN", 0, ConnectorType.BOOLEAN);
        this.OUT = new FlowchartOutputConnector(this, "OUT", 0, ConnectorType.FLOAT);
        this.AppendConnectors([this.IN], [this.OUT]);
    }

    public PopulateProperyGrid(tbody:HTMLTableSectionElement):boolean
    {
        this.numberTRUEHTMLInput=InputFloatNumber(tbody, NUMBER_TRUE, this.configurationData);
        this.numberFALSEHTMLInput=InputFloatNumber(tbody, NUMBER_FALSE, this.configurationData);
        return true;
    }

    public SavePropertyGrid(tbody:HTMLTableSectionElement){
        if(this.numberFALSEHTMLInput==null || this.numberTRUEHTMLInput==null) return;
        this.cfg_setValue(NUMBER_TRUE, this.numberTRUEHTMLInput.valueAsNumber);
        this.cfg_setValue(NUMBER_FALSE, this.numberFALSEHTMLInput.valueAsNumber);
    }

    protected SerializeFurtherProperties(ctx:SerializeContextAndAdressMap):void{
        ctx.ctx.writeF32(this.cfg_getValue(NUMBER_TRUE, 1));
        ctx.ctx.writeF32(this.cfg_getValue(NUMBER_FALSE, 0));
        return;
    }

    public OnSimulationStep(ctx:SimulationContext){
        let i = ctx.GetBoolean(this.IN);
        ctx.SetFloat(this.OUT, i?this.cfg_getValue(NUMBER_TRUE, 1):this.cfg_getValue(NUMBER_FALSE, 0));
    }
}

export class Int2BoolConvert extends FlowchartOperator {
    private IN:FlowchartInputConnector;
    private OUT:FlowchartOutputConnector;
    
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.IN = new FlowchartInputConnector(this, "IN", 0, ConnectorType.INTEGER);
        this.OUT = new FlowchartOutputConnector(this, "OUT", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([this.IN], [this.OUT]);
    }

    public OnSimulationStep(ctx:SimulationContext){
        let i = ctx.GetInteger(this.IN);
        ctx.SetBoolean(this.OUT, i!=0);
    }
}



export class Int2FloatConvert extends FlowchartOperator {
    private IN:FlowchartInputConnector;
    private OUT:FlowchartOutputConnector;
    
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.IN = new FlowchartInputConnector(this, "IN", 0, ConnectorType.INTEGER);
        this.OUT = new FlowchartOutputConnector(this, "OUT", 0, ConnectorType.FLOAT);
        this.AppendConnectors([this.IN], [this.OUT]);
    }

    public OnSimulationStep(ctx:SimulationContext){
        let i = ctx.GetInteger(this.IN);
        ctx.SetFloat(this.OUT, i);
    }
}


export class Int2ColorConvert extends FlowchartOperator {
    private IN:FlowchartInputConnector;
    private OUT:FlowchartOutputConnector;
    
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.IN = new FlowchartInputConnector(this, "IN", 0, ConnectorType.INTEGER);
        this.OUT = new FlowchartOutputConnector(this, "OUT", 0, ConnectorType.COLOR);
        this.AppendConnectors([this.IN], [this.OUT]);
    }

    public OnSimulationStep(ctx:SimulationContext){
        let currentInputValue = ctx.GetInteger(this.IN);
        ctx.SetColor(this.OUT, currentInputValue?"RED": "GREY");//TODO: Has to be improved
    }
}


export class Float2IntConvert extends FlowchartOperator {
    private IN:FlowchartInputConnector;
    private OUT:FlowchartOutputConnector;
    
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.IN = new FlowchartInputConnector(this, "IN", 0, ConnectorType.FLOAT);
        this.OUT = new FlowchartOutputConnector(this, "OUT", 0, ConnectorType.INTEGER);
        this.AppendConnectors([this.IN], [this.OUT]);
    }

    public OnSimulationStep(ctx:SimulationContext){
        let i = ctx.GetFloat(this.IN);
        ctx.SetInteger(this.OUT, i);
    }
}



export class Basic_TONOperator extends FlowchartOperator {
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

export class Basic_TOFOperator extends FlowchartOperator {
  
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
        else if(currentInputValue==true){
            this.inputNegativeEdge=0;
        }
        this.lastInputValue=currentInputValue;
        let elapsed = (now-this.inputNegativeEdge);
        elapsed=Math.min(elapsed, presetTime_msecs)
        ctx.SetBoolean(this.output, currentInputValue || (elapsed<presetTime_msecs));
        ctx.SetInteger(this.outputElapsedTime_msecs, elapsed);
    }
}


export class Arithmetic_LIMITOperator extends FlowchartOperator {
    protected Minimum:FlowchartInputConnector;
    protected Input:FlowchartInputConnector;
    protected Maximum:FlowchartInputConnector;
    protected Output:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.Minimum = new FlowchartInputConnector(this, "Minimum", 0, ConnectorType.FLOAT);
        this.Input = new FlowchartInputConnector(this, "Input", 1, ConnectorType.FLOAT);
        this.Maximum = new FlowchartInputConnector(this, "Maximum", 2, ConnectorType.FLOAT);
        this.Output = new FlowchartOutputConnector(this, "Output", 0, ConnectorType.FLOAT);
        this.AppendConnectors([this.Minimum, this.Input, this.Maximum], [this.Output]);
    }

    public OnSimulationStep(ctx: SimulationContext): void {
        let i = ctx.GetInteger(this.Input);
        let min = ctx.GetInteger(this.Minimum);
        let max = ctx.GetInteger(this.Minimum);
        ctx.SetInteger(this.Output, i>max?max:i<min?min:i);
    }
}

export class Arithmetic_LIMITMONITOROperator extends FlowchartOperator {
    protected Minimum:FlowchartInputConnector;
    protected Input:FlowchartInputConnector;
    protected Maximum:FlowchartInputConnector;
    protected Hysterese:FlowchartInputConnector;
    protected LLE:FlowchartOutputConnector;
    protected ULE:FlowchartOutputConnector;
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.Minimum = new FlowchartInputConnector(this, "Minimum", 0, ConnectorType.FLOAT);
        this.Input = new FlowchartInputConnector(this, "Input", 1, ConnectorType.FLOAT);
        this.Maximum = new FlowchartInputConnector(this, "Maximum", 2, ConnectorType.FLOAT);
        this.Hysterese = new FlowchartInputConnector(this, "Hysterese", 3, ConnectorType.FLOAT);
        this.LLE = new FlowchartOutputConnector(this, "LLE", 0, ConnectorType.BOOLEAN);
        this.ULE = new FlowchartOutputConnector(this, "ULE", 1, ConnectorType.BOOLEAN);
        this.AppendConnectors([this.Minimum, this.Input, this.Maximum], [this.LLE, this.ULE]);
    }

    public OnSimulationStep(ctx: SimulationContext): void {
        let i = ctx.GetInteger(this.Input);
        let min = ctx.GetInteger(this.Minimum);
        let max = ctx.GetInteger(this.Minimum);
        let h = ctx.GetInteger(this.Hysterese);
        if(i>max){
            ctx.SetBoolean(this.ULE, true);
        }else if(i<=max-h){
            ctx.SetBoolean(this.ULE, false);
        }
        if(i<min){
            ctx.SetBoolean(this.LLE, true);
        } else if(i>=min+h){
            ctx.SetBoolean(this.LLE, false);
        }
    }
}


export class Arithmetic_GreaterThanOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.FLOAT);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.FLOAT);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([A, B], [C]);
    }
}

export class Arithmetic_LessThanOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.FLOAT);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.FLOAT);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([A, B], [C]);
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
        this.value1HTMLInput=InputIntegerNumber(tbody, -32768, 32767, "Value1", this.configurationData);
        this.value1HTMLInput=InputIntegerNumber(tbody, -32768, 32767, "Value2", this.configurationData);
        this.value1HTMLInput=InputIntegerNumber(tbody, -32768, 32767, "Value3", this.configurationData);
        this.color1HTMLInput=InputColor(tbody, "Color1", this.configurationData);
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
        let colorNum=ColorDomString2ColorNum(colorString);
        ctx.ctx.writeU32(colorNum);
        return;
    }
}

const KP="Kp";
const Tn="Tn(msecs)";
const Tv="Tv(msecs)";
const MIN_OUTPUT="Min Output";
const MAX_OUTPUT="Max Output";
const DIRECTION  = "Direction";

export class Control_PID extends FlowchartOperator {
    private inputSetpoint:FlowchartInputConnector;
    private inputFeedback:FlowchartInputConnector;
    private output:FlowchartOutputConnector;
    

    private kpHTMLInput:HTMLInputElement|null=null;
    private tnHTMLInput:HTMLInputElement|null=null;
    private tvHTMLInput:HTMLInputElement|null=null;
    private minOutputHTMLInput:HTMLInputElement|null=null;
    private maxOutputHTMLInput:HTMLInputElement|null=null;
    private directionHTMLSelect:HTMLSelectElement|null=null;
    public PopulateProperyGrid(tbody:HTMLTableSectionElement):boolean
    {
        this.kpHTMLInput=InputFloatNumber(tbody, KP, this.configurationData);
        this.tnHTMLInput=InputIntegerNumber(tbody, 0, 100000, Tn, this.configurationData);
        this.tvHTMLInput=InputIntegerNumber(tbody, 0, 100000, Tv, this.configurationData);
        this.minOutputHTMLInput=InputFloatNumber(tbody, MIN_OUTPUT, this.configurationData);
        this.maxOutputHTMLInput=InputFloatNumber(tbody, MAX_OUTPUT, this.configurationData);
        this.directionHTMLSelect=InputSelect(tbody, [new StringNumberTuple("Direct", 0), new StringNumberTuple("Inverse", 1)], DIRECTION, this.configurationData);
        return true;
    }

    public SavePropertyGrid(tbody:HTMLTableSectionElement){
        if(this.directionHTMLSelect==null) return;
        this.cfg_setValue(KP, this.kpHTMLInput!.valueAsNumber);
        this.cfg_setValue(Tn, parseInt(this.tnHTMLInput!.value));
        this.cfg_setValue(Tv, parseInt(this.tvHTMLInput!.value));
        this.cfg_setValue(MIN_OUTPUT, this.minOutputHTMLInput!.valueAsNumber);
        this.cfg_setValue(MAX_OUTPUT, this.maxOutputHTMLInput!.valueAsNumber);
        this.cfg_setValue(DIRECTION, parseInt(this.directionHTMLSelect.value));
    }

    protected SerializeFurtherProperties(ctx:SerializeContextAndAdressMap):void{
        ctx.ctx.writeF32(this.cfg_getValue(KP, 0));
        ctx.ctx.writeU32(this.cfg_getValue(Tn, 0));
        ctx.ctx.writeU32(this.cfg_getValue(Tv, 0));
        ctx.ctx.writeF32(this.cfg_getValue(MIN_OUTPUT, 0));
        ctx.ctx.writeF32(this.cfg_getValue(MAX_OUTPUT, 0));
        ctx.ctx.writeU32(this.cfg_getValue(DIRECTION, 0));
        return;
    }


    constructor(parent: Flowchart, caption: string, ti:TypeInfo, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, ti, configurationData);
        this.inputSetpoint = new FlowchartInputConnector(this, "Setpoint", 1, ConnectorType.FLOAT);
        this.inputFeedback = new FlowchartInputConnector(this, "Feedback", 0, ConnectorType.FLOAT);
        this.output = new FlowchartOutputConnector(this, "Out", 1,ConnectorType.INTEGER);
        this.AppendConnectors([this.inputSetpoint, this.inputFeedback], [this.output]);
    }
}