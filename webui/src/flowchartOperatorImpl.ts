import {FlowchartOperator} from "./flowchartOperator";
import {Flowchart, ConnectorType} from "./flowchart";
import {FlowchartInputConnector, FlowchartOutputConnector} from "./flowchartConnector";

export class ANDOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string) {
        super(parent, "AND", caption);
        let A = new FlowchartInputConnector(this, "A", ConnectorType.BOOLEAN);
        let B = new FlowchartInputConnector(this, "B", ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", ConnectorType.BOOLEAN);
        this.AppendConnectors([A, B], [C]);
    }
}

export class OROperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string) {
        super(parent, "OR", caption);
        let A = new FlowchartInputConnector(this, "A", ConnectorType.BOOLEAN);
        let B = new FlowchartInputConnector(this, "B", ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", ConnectorType.BOOLEAN);
        this.AppendConnectors([A, B], [C]);
    }
}

export class ADDOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string) {
        super(parent, "OR", caption);
        let A = new FlowchartInputConnector(this, "A", ConnectorType.INTEGER);
        let B = new FlowchartInputConnector(this, "B", ConnectorType.INTEGER);
        let C = new FlowchartOutputConnector(this, "C", ConnectorType.INTEGER);
        this.AppendConnectors([A, B], [C]);
    }
}

export class RSOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string) {
        super(parent, "RS", caption);
        let R = new FlowchartInputConnector(this, "R", ConnectorType.BOOLEAN);
        let S = new FlowchartInputConnector(this, "S", ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", ConnectorType.BOOLEAN);
        this.AppendConnectors([R, S], [C]);
    }
}

export class NotOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string) {
        super(parent, "Not", caption);
        let A = new FlowchartInputConnector(this, "A", ConnectorType.BOOLEAN);
        let C = new FlowchartOutputConnector(this, "C", ConnectorType.BOOLEAN);
        this.AppendConnectors([A], [C]);
    }
}

export class InputsOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string) {
        super(parent, "Inputs", caption);
        let _1 = new FlowchartOutputConnector(this, "I1", ConnectorType.BOOLEAN);
        let _2 = new FlowchartOutputConnector(this, "I2", ConnectorType.BOOLEAN);
        let _3 = new FlowchartOutputConnector(this, "I3", ConnectorType.BOOLEAN);
        let _4 = new FlowchartOutputConnector(this, "I4", ConnectorType.BOOLEAN);
        this.AppendConnectors([], [_1, _2, _3, _4]);
    }
}

export class OutputsOperator extends FlowchartOperator {
    constructor(parent: Flowchart, caption: string) {
        super(parent, "Inputs", caption);
        let _1 = new FlowchartInputConnector(this, "O1", ConnectorType.BOOLEAN);
        let _2 = new FlowchartInputConnector(this, "O2", ConnectorType.BOOLEAN);
        let _3 = new FlowchartInputConnector(this, "O3", ConnectorType.BOOLEAN);
        let _4 = new FlowchartInputConnector(this, "O4", ConnectorType.BOOLEAN);
        this.AppendConnectors([_1, _2, _3, _4], []);
    }
}

//Just to break circles programatically
export class StorageInputOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string) {
        super(parent, "StorageInput", caption);
        let O = new FlowchartOutputConnector(this, "In", ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
    }
}

export class StorageOutputOperator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string) {
        super(parent, "StorageOutput", caption);
        let I = new FlowchartInputConnector(this, "Out", ConnectorType.BOOLEAN);
        this.AppendConnectors([I], []);
    }
}

export class Constant_1_Operator extends FlowchartOperator {
    public StorageId:string;
    constructor(parent: Flowchart, caption: string) {
        super(parent, "Constant", caption);
        let O = new FlowchartOutputConnector(this, "1", ConnectorType.BOOLEAN);
        this.AppendConnectors([], [O]);
    }
}