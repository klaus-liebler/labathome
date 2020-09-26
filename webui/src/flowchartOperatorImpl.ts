import {FlowchartOperator, TypeInfo, PositionType, SingletonType} from "./FlowchartOperator";
import {Flowchart, KeyValueTuple} from "./Flowchart";
import {FlowchartInputConnector, FlowchartOutputConnector, ConnectorType} from "./FlowchartConnector";
import { SerializeContext } from "./FlowchartSerializer";


export class ANDOperator extends FlowchartOperator {
  
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(1, "AND", PositionType.Default, SingletonType.Default), configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.BOOLEAN);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([A, B], [C]);
    }
}

export class OROperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(2, "OR", PositionType.Default, SingletonType.Default), configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.BOOLEAN);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", 0,ConnectorType.BOOLEAN);
        this.AppendConnectors([A, B], [C]);
    }
}

export class ADDOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(3, "ADD", PositionType.Default, SingletonType.Default), configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}

export class MULTIPLYOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(4, "MULTIPLY", PositionType.Default, SingletonType.Default), configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}

export class MAXOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(5, "MAX", PositionType.Default, SingletonType.Default), configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}

export class MINOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(6, "MIN", PositionType.Default, SingletonType.Default), configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", 1, ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}

export class RSOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(7, "RS", PositionType.Default, SingletonType.Default), configurationData);
        let R = new FlowchartInputConnector(this, "R", 0, ConnectorType.BOOLEAN);
        let S = new FlowchartInputConnector(this, "S", 1, ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([R, S], [C]);
    }
}

export class NotOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(8, "Not", PositionType.Default, SingletonType.Default), configurationData);
        let A = new FlowchartInputConnector(this, "A", 0, ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([A], [C]);
    }

}

export class GreenButtonOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(9, "GreenButton", PositionType.Input, SingletonType.Singleton), configurationData);
        let O = new FlowchartOutputConnector(this, "IsPressed", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class EncoderButtonOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(10, "EncoderButton", PositionType.Input, SingletonType.Singleton), configurationData);
        let O = new FlowchartOutputConnector(this, "IsPressed", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class RedButtonOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(11, "RedButton", PositionType.Input, SingletonType.Singleton), configurationData);
        let O = new FlowchartOutputConnector(this, "IsPressed", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class MovementSensorOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(12, "MovementSensor", PositionType.Input, SingletonType.Singleton), configurationData);
        let O = new FlowchartOutputConnector(this, "Movement", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class TempSensorOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(13, "TempSensor", PositionType.Input, SingletonType.Singleton), configurationData);
        let O = new FlowchartOutputConnector(this, "Temperatur", 0, ConnectorType.FLOAT);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class RelayOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(14, "Relay", PositionType.Output, SingletonType.Singleton), configurationData);
        let conn = new FlowchartInputConnector(this, "Relay", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class RedLedOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(15, "RedLed", PositionType.Output, SingletonType.Singleton), configurationData);
        let conn = new FlowchartInputConnector(this, "LED", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class YellowLedOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(16, "YellowLed", PositionType.Output, SingletonType.Singleton), configurationData);
        let conn = new FlowchartInputConnector(this, "LED", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class GreenLedOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(17, "GreenLed", PositionType.Output, SingletonType.Singleton), configurationData);
        let conn = new FlowchartInputConnector(this, "LED", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class ConstTRUEOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(18, "ConstTRUE", PositionType.Input, SingletonType.Default), configurationData);
        let O = new FlowchartOutputConnector(this, "TRUE", 0, ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class ConstINTOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(19, "ConstINT", PositionType.Input, SingletonType.Default), configurationData);
        let O = new FlowchartOutputConnector(this, "Out", 0, ConnectorType.INTEGER);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }

    public PopulateProperyGrid(parent:HTMLTableElement):boolean
    {
        PropertyGridHelpers.Number(parent, "Constant", -32768, 32767);
        return true;
    }

    protected SerializeFurtherProperties(ctx:SerializeContext):void{
        this.serializeS32(ctx, 42)
        return;
    }
}

export class KampmannXYZOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string, configurationData:KeyValueTuple[]|null) {
        super(parent, caption, new TypeInfo(1000, "KampmannXYZ", PositionType.Default, SingletonType.Default), configurationData);
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
    public static Number(table:HTMLTableElement, key:string, min:number, max:number)
    {
        let tr=Flowchart.Html(table, "tr", [],["develop-propertygrid-tr"]);
        Flowchart.Html(tr, "td", [],["develop-propertygrid-td"], key);
        let inputContainer = Flowchart.Html(tr, "td", [],["develop-propertygrid-td"]);
        Flowchart.Html(inputContainer, "input", ["type", "number", "min", ""+Math.round(min), "max", ""+Math.round(max)])
    }
}
