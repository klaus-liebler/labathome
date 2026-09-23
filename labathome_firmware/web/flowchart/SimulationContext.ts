import { FlowchartInputConnector, FlowchartOutputConnector } from "./FlowchartConnector";


export interface SimulationContext {
    GetMillis(): number;
    SetBoolean(outConn: FlowchartOutputConnector, value: boolean): void;
    SetInteger(outConn: FlowchartOutputConnector, value: number): void;
    SetFloat(outConn: FlowchartOutputConnector, value: number): void;
    SetColor(outConn: FlowchartOutputConnector, value: string): void;
    GetBoolean(inConn: FlowchartInputConnector): boolean;
    GetInteger(inConn: FlowchartInputConnector): number;
    GetFloat(inConn: FlowchartInputConnector): number;
    GetColor(inConn: FlowchartInputConnector): string;
}
;
