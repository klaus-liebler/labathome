
/*
Dieser Block sollte zur Kompilierzeit ins Binärformat gebracht werden und dann sowohl als JSON als auch als ausführbares Bin-File auf dem Labathome zur Verfügung stehen.
Problem: Der Compiler möchte zur Zeit jene Datenstruktur, die im Browser zur Laufzeit zur verfügung steht. Diese Datenstruktur enthält SVG / DOM Themen
Das steht zur Compilierzeit in einer NodeJS-Umgebung nicht zur Verfügung
Aufgabe: Der 


*/
import { FlowchartData, OperatorData, LinkData } from "../web/typescript/flowchart/FlowchartData";
export const data: FlowchartData = {
    operators: [
        {
            index: 0,
            caption: "RedButton",
            globalTypeIndex: 33,
            posX: 10,
            posY: 10,
            configurationData: null,
        },
        {
            index: 1,
            caption: "GreenButton",
            globalTypeIndex: 30,
            posX: 10,
            posY: 150,
            configurationData: null,
        },
        {
            index: 2,
            caption: "AND",
            globalTypeIndex: 1,
            posX: 250,
            posY: 10,
            configurationData: null,
        },
        {
            index: 3,
            caption: "RedLed",
            globalTypeIndex: 49,
            posX: 500,
            posY: 10,
            configurationData: null,
        },
    ],
    links: [
        {
            color: "black",
            fromOperatorIndex: 0,
            fromOutput: 0,
            toOperatorIndex: 2,
            toInput: 0
        },
        {
            color: "black",
            fromOperatorIndex: 1,
            fromOutput: 0,
            toOperatorIndex: 2,
            toInput: 1
        },
        {
            color: "black",
            fromOperatorIndex: 2,
            fromOutput: 0,
            toOperatorIndex: 3,
            toInput: 0
        },
    ]
};

export default data;