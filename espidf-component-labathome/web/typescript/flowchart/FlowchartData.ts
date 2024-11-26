import { KeyValueTuple } from "../utils/common";

export interface FlowchartData {
    operators: OperatorData[];
    links: LinkData[];
}

export interface OperatorData {
    globalTypeIndex: number;
    caption: string;
    index: number;
    posX: number;
    posY: number;
    configurationData: KeyValueTuple[] | null;
}



export interface LinkData {
    color: string;
    fromOperatorIndex: number;
    fromOutput: number;
    toOperatorIndex: number;
    toInput: number;
}