import {FlowchartOperator, TypeInfo, PositionType, SingletonType} from "./FlowchartOperator";
import {Flowchart} from "./Flowchart";
import {FlowchartInputConnector, FlowchartOutputConnector, ConnectorType} from "./FlowchartConnector";
import { SerializeContext } from "./FlowchartExporter";


export class ANDOperator extends FlowchartOperator {
  
    constructor(parent: Flowchart, caption: string) {
        super(parent, caption, new TypeInfo(1, PositionType.Default, SingletonType.Default));
        let A = new FlowchartInputConnector(this, "A", ConnectorType.BOOLEAN);
        let B = new FlowchartInputConnector(this, "B", ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", ConnectorType.BOOLEAN);
        this.AppendConnectors([A, B], [C]);
    }
}

export class OROperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string) {
        super(parent, caption, new TypeInfo(2, PositionType.Default, SingletonType.Default));
        let A = new FlowchartInputConnector(this, "A", ConnectorType.BOOLEAN);
        let B = new FlowchartInputConnector(this, "B", ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", ConnectorType.BOOLEAN);
        this.AppendConnectors([A, B], [C]);
    }
}

export class ADDOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string) {
        super(parent, caption, new TypeInfo(3, PositionType.Default, SingletonType.Default));
        let A = new FlowchartInputConnector(this, "A", ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}

export class MULTOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string) {
        super(parent, caption, new TypeInfo(4, PositionType.Default, SingletonType.Default));
        let A = new FlowchartInputConnector(this, "A", ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}

export class MAXOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string) {
        super(parent, caption, new TypeInfo(5, PositionType.Default, SingletonType.Default));
        let A = new FlowchartInputConnector(this, "A", ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}

export class MINOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string) {
        super(parent, caption, new TypeInfo(6, PositionType.Default, SingletonType.Default));
        let A = new FlowchartInputConnector(this, "A", ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}

export class RSOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string) {
        super(parent, caption, new TypeInfo(7, PositionType.Default, SingletonType.Default));
        let R = new FlowchartInputConnector(this, "R", ConnectorType.BOOLEAN);
        let S = new FlowchartInputConnector(this, "S", ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", ConnectorType.BOOLEAN);
        this.AppendConnectors([R, S], [C]);
    }
}

export class NotOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string) {
        super(parent, caption, new TypeInfo(8, PositionType.Default, SingletonType.Default));
        let A = new FlowchartInputConnector(this, "A", ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", ConnectorType.BOOLEAN);
        this.AppendConnectors([A], [C]);
    }

}


export class GreenButtonOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string) {
        super(parent, caption, new TypeInfo(9, PositionType.Input, SingletonType.Singleton));
        let O = new FlowchartOutputConnector(this, "IsPressed", ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class EncoderButtonOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string) {
        super(parent, caption, new TypeInfo(10, PositionType.Input, SingletonType.Singleton));
        let O = new FlowchartOutputConnector(this, "IsPressed", ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class RedButtonOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string) {
        super(parent, caption, new TypeInfo(11, PositionType.Input, SingletonType.Singleton));
        let O = new FlowchartOutputConnector(this, "IsPressed", ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class MoveSensorOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string) {
        super(parent, caption, new TypeInfo(12, PositionType.Input, SingletonType.Singleton));
        let O = new FlowchartOutputConnector(this, "Movement", ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class TempSensorOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string) {
        super(parent, caption, new TypeInfo(13, PositionType.Input, SingletonType.Singleton));
        let O = new FlowchartOutputConnector(this, "Temperatur", ConnectorType.FLOAT);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class RelayOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string) {
        super(parent, caption, new TypeInfo(14, PositionType.Output, SingletonType.Singleton));
        let conn = new FlowchartInputConnector(this, "Relay", ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class RedLedOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string) {
        super(parent, caption, new TypeInfo(15, PositionType.Output, SingletonType.Singleton));
        let conn = new FlowchartInputConnector(this, "LED", ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class YellowLedOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string) {
        super(parent, caption, new TypeInfo(16, PositionType.Output, SingletonType.Singleton));
        let conn = new FlowchartInputConnector(this, "LED", ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class GreenLedOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string) {
        super(parent, caption, new TypeInfo(17, PositionType.Output, SingletonType.Singleton));
        let conn = new FlowchartInputConnector(this, "LED", ConnectorType.BOOLEAN);
        this.AppendConnectors([conn], []);
        this.StorageId="4711";
    }
}

export class ConstTRUEOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string) {
        super(parent, caption, new TypeInfo(18, PositionType.Input, SingletonType.Default));
        let O = new FlowchartOutputConnector(this, "TRUE", ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
        this.StorageId="4711";
    }
}

export class ConstINTOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string) {
        super(parent, caption, new TypeInfo(19, PositionType.Input, SingletonType.Default));
        let O = new FlowchartOutputConnector(this, "Out", ConnectorType.INTEGER);
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
