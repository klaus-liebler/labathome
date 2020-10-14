import {FlowchartOperator, TypeInfo, PositionType, SingletonType} from "./FlowchartOperator";
import {Flowchart, KeyValueTuple} from "./Flowchart";
import {FlowchartInputConnector, FlowchartOutputConnector, ConnectorType} from "./FlowchartConnector";
import { SerializeContextAndAdressMap } from "./FlowchartCompiler";
import {$} from "./../Utils"

export class Logic_ANDOperator extends FlowchartOperator {
  
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(1, "Logic_ANDOperator", PositionType.Default, SingletonType.Default), configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.BOOLEAN);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([A, B], [C]);
    }
}



export class Logic_OROperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(2, "Logic_OROperator", PositionType.Default, SingletonType.Default), configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.BOOLEAN);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", 0,ConnectorType.BOOLEAN);
        this.AppendConnectors([A, B], [C]);
    }
}

export class Arithmetic_ADDOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(3, "Arithmetic_ADDOperator", PositionType.Default, SingletonType.Default), configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}

export class Arithmetic_MULTIPLYOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(4, "Arithmetic_MULTIPLYOperator", PositionType.Default, SingletonType.Default), configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}

export class Arithmetic_MAXOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(5, "Arithmetic_MAXOperator", PositionType.Default, SingletonType.Default), configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}



export class Arithmetic_MINOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(6, "Arithmetic_MINOperator", PositionType.Default, SingletonType.Default), configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}

export class Logic_RSOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(7, "Logic_RSOperator", PositionType.Default, SingletonType.Default), configurationData);
        let R = new FlowchartInputConnector(this, "R", 0, ConnectorType.BOOLEAN);
        let S = new FlowchartInputConnector(this, "S", 1, ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([R, S], [C]);
    }
}

export class Logic_NotOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(8, "Logic_NotOperator", PositionType.Default, SingletonType.Default), configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([A], [C]);
    }

}

export class Input_GreenButtonOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(9, "Input_GreenButtonOperator", PositionType.Input, SingletonType.Singleton), configurationData);
        let O = new FlowchartOutputConnector(this, "IsPressed", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class Input_EncoderButtonOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(10, "Input_EncoderButtonOperator", PositionType.Input, SingletonType.Singleton), configurationData);
        let O = new FlowchartOutputConnector(this, "IsPressed", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class Input_RedButtonOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(11, "Input_RedButtonOperator", PositionType.Input, SingletonType.Singleton), configurationData);
        let O = new FlowchartOutputConnector(this, "IsPressed", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class Sensor_MovementOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(12, "Sensor_MovementOperator", PositionType.Input, SingletonType.Singleton), configurationData);
        let O = new FlowchartOutputConnector(this, "Movement", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class Sensor_AmbientTemperatureOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(13, "Sensor_AmbientTemperatureOperator", PositionType.Input, SingletonType.Singleton), configurationData);
        let O = new FlowchartOutputConnector(this, "Temperatur", 0, ConnectorType.INTEGER);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class Output_RelayOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(14, "Output_RelayOperator", PositionType.Output, SingletonType.Singleton), configurationData);
        let conn = new FlowchartInputConnector(this, "Relay", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class Output_RedLedOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(15, "Output_RedLedOperator", PositionType.Output, SingletonType.Singleton), configurationData);
        let conn = new FlowchartInputConnector(this, "LED", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class Output_YellowLedOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(16, "Output_YellowLedOperator", PositionType.Output, SingletonType.Singleton), configurationData);
        let conn = new FlowchartInputConnector(this, "LED", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class Output_GreenLedOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(17, "Output_GreenLedOperator", PositionType.Output, SingletonType.Singleton), configurationData);
        let conn = new FlowchartInputConnector(this, "LED", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class Logic_ConstTRUEOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(18, "Logic_ConstTRUEOperator", PositionType.Input, SingletonType.Default), configurationData);
        let O = new FlowchartOutputConnector(this, "TRUE", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

const CONSTANT = "Constant";
export class Arithmetic_ConstINTOperator extends FlowchartOperator {
    
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(19, "Arithmetic_ConstINTOperator", PositionType.Input, SingletonType.Default), configurationData);
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

export class Timing_TONOperator extends FlowchartOperator {
  
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(20, "Timing_TONOperator", PositionType.Default, SingletonType.Default), configurationData);
        let A = new FlowchartInputConnector(this, "TRIGGER", 0, ConnectorType.BOOLEAN);
        let B = new FlowchartInputConnector(this, "PT_MS", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "OUT", 0, ConnectorType.BOOLEAN);
        let D = new FlowchartOutputConnector(this, "ET_MS", 0, ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C,D]);
    }
}

export class Timing_TOFOperator extends FlowchartOperator {
  
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(21, "Timing_TOFOperator", PositionType.Default, SingletonType.Default), configurationData);
        let A = new FlowchartInputConnector(this, "TRIGGER", 0, ConnectorType.BOOLEAN);
        let B = new FlowchartInputConnector(this, "PresetTimeMS", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "OUT", 0, ConnectorType.BOOLEAN);
        let D = new FlowchartOutputConnector(this, "ElapsedTimeMS", 0, ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C,D]);
    }
}

export class Arithmetic_GreaterThanOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(22, "Arithmetic_GreaterThanOperator", PositionType.Default, SingletonType.Default), configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([A, B], [C]);
    }
}

export class Arithmetic_LessThanOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(23, "Arithmetic_LessThanOperator", PositionType.Default, SingletonType.Default), configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([A, B], [C]);
    }
}

export class Sensor_AmbientLight extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(24, "Sensor_AmbientLight", PositionType.Input, SingletonType.Singleton), configurationData);
        let O = new FlowchartOutputConnector(this, "Lux", 0, ConnectorType.INTEGER);
        this.AppendConnectors([], [O]);
    }
}

export class Sensor_HeaterTemperature extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(25, "Sensor_HeaterTemperature", PositionType.Input, SingletonType.Singleton), configurationData);
        let O = new FlowchartOutputConnector(this, "Degrees", 0, ConnectorType.INTEGER);
        this.AppendConnectors([], [O]);
    }
}

export class Custom_KampmannXYZOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(1000, "Custom_KampmannXYZOperator", PositionType.Default, SingletonType.Default), configurationData);
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
}
