import {FlowchartOperator, PositionType } from "./FlowchartOperator";
import {ConnectorType } from "./FlowchartConnector";
import { NodeWrapper, TopologicalSortDFS } from "./TopologicalSorfDFS";
import { SerializeContext } from "./SerializeContext";
import { FlowchartLink } from "./FlowchartLink";

export interface SerializeContextAndAdressMap {
    typeIndex2globalConnectorIndex2adressOffset: Map<number, Map<number, number>>;
    ctx: SerializeContext;
}

export interface HashAndBufAndMaps{
    hash:number;
    buf: ArrayBuffer;
    typeIndex2globalConnectorIndex2adressOffset:Map<number, Map<number, number>>,
    typeIndex2adressOffset2ListOfLinks:Map<number, Map<number, Array<FlowchartLink>>>,
    typeIndex2maxOffset:Map<number, number>,
}

export interface HashAndBuf{
    hash:number;
    buf: ArrayBuffer;
}

export interface SortedOperatorsAndMaps{
    sortedOperators:FlowchartOperator[];
    typeIndex2globalConnectorIndex2adressOffset:Map<number, Map<number, number>>,
    typeIndex2adressOffset2ListOfLinks:Map<number, Map<number, Array<FlowchartLink>>>,
    typeIndex2maxOffset:Map<number, number>,
}

export interface Maps{
    typeIndex2globalConnectorIndex2adressOffset:Map<number, Map<number, number>>,
    typeIndex2adressOffset2ListOfLinks:Map<number, Map<number, Array<FlowchartLink>>>,
    typeIndex2maxOffset:Map<number, number>,
}

export class FlowchartCompiler {
    
    public constructor(private index2operator:Map<number,FlowchartOperator>)
    {
       
    }

    private sortOperators():FlowchartOperator[]{
        let index2wrappedOperator = new Map<number, NodeWrapper<FlowchartOperator>>();
        this.index2operator.forEach((v, k, m) => {
            index2wrappedOperator.set(v.GlobalOperatorIndex, new NodeWrapper<FlowchartOperator>(v));
        });
        let wrappedOutputOperators: NodeWrapper<FlowchartOperator>[] = [];
        for (let i of index2wrappedOperator.values()) {
            //Stelle für jede "gewrapte Node" fest, welche Operatoren von Ihr abhängig sind
            let dependents = new Set<NodeWrapper<FlowchartOperator>>();
            for (const inputkv of i.Payload.InputsKVIt) {
                for (const linkkv of inputkv[1].LinksKVIt) {
                    let dependentOperator = linkkv[1].From.Parent;
                    let dependentWrappedNode = index2wrappedOperator.get(dependentOperator.GlobalOperatorIndex);
                    if (!dependentWrappedNode)
                        throw new Error("Implementation Error: dependentWrappedNode is undefined");
                    dependents.add(dependentWrappedNode);
                }
            }
            dependents.forEach(e => i.DependendNodes.push(e));
            //füge alle mit Typ "Output" einer Liste hinzu
            if (i.Payload.TypeInfo.Position == PositionType.Output) wrappedOutputOperators.push(i);
        }

        let algorithm = new TopologicalSortDFS<FlowchartOperator>();
        let sortedList = algorithm.sort(wrappedOutputOperators);
        return sortedList.map((e) => e.Payload)
    }

    public CompileForSimulation():SortedOperatorsAndMaps{
        let sortedOperators = this.sortOperators();
        for (const key in sortedOperators) {
            let value = sortedOperators[key];
            value.SetDebugInfoText("Sequence " + key);
        }
        let maps=this.createLookupMaps(sortedOperators);
        return {
            sortedOperators:sortedOperators,
            typeIndex2globalConnectorIndex2adressOffset:maps.typeIndex2globalConnectorIndex2adressOffset,
            typeIndex2adressOffset2ListOfLinks:maps.typeIndex2adressOffset2ListOfLinks,
            typeIndex2maxOffset:maps.typeIndex2maxOffset,
        };
    }

    public Compile(): HashAndBufAndMaps {
        let sortedOperators = this.sortOperators();
        for (const key in sortedOperators) {
            let value = sortedOperators[key];
            value.SetDebugInfoText("Sequence " + key);
        }
        let maps=this.createLookupMaps(sortedOperators);
        let hashAndBuf= this.serialize(sortedOperators, maps);

        let dv = new DataView(hashAndBuf.buf);
        let code: String = "const uint8_t code[] = {"
        for (let i = 0; i < dv.byteLength; i++) {
            code += "0x" + dv.getUint8(i).toString(16) + ", ";
        }
        code += "};//"+dv.byteLength+"bytes";
        console.log(code);

        return {
            hash:hashAndBuf.hash,
            buf: hashAndBuf.buf,
            typeIndex2globalConnectorIndex2adressOffset:maps.typeIndex2globalConnectorIndex2adressOffset,
            typeIndex2adressOffset2ListOfLinks:maps.typeIndex2adressOffset2ListOfLinks,
            typeIndex2maxOffset:maps.typeIndex2maxOffset,
        };
    }

    private createLookupMaps(operators:FlowchartOperator[]):Maps{
        //Speichert separat für jeden Datentyp (Bool, int, float, color,...), welcher GlobalConnectorIndex auf welchen bei 2 beginnend fortlaufenden Adress-Offset gemapped wird
        //wir beginnen bei 2, weil unbeschaltete Outputs auf 0 schreiben und unbeschaltete Inputs von 1 lesen.
        let typeIndex2globalConnectorIndex2adressOffset = new Map<number, Map<number, number>>(); //globalConnectorIndex_Outputs 2 variableAdress
        let typeIndex2maxOffset = new Map<number, number>();
        let typeIndex2adressOffset2ListOfLinks = new Map<number, Map<number, Array<FlowchartLink>>>();
        for (let type in ConnectorType) {
            if (!isNaN(Number(type))) {
                typeIndex2globalConnectorIndex2adressOffset.set(Number(type), new Map<number, number>());
                typeIndex2maxOffset.set(Number(type), 2);
                typeIndex2adressOffset2ListOfLinks.set(Number(type), new Map<number,Array<FlowchartLink>>());
            }
        }
        for (const operator of operators) {
            for (const outputKV of operator.OutputsKVIt) {
                if (outputKV[1].LinksLength == 0) {
                    //unconnected output -->writes to memory adress zero of the respective data type
                    typeIndex2globalConnectorIndex2adressOffset.get(outputKV[1].Type)!.set(outputKV[1].GlobalConnectorIndex, 0);
                }
                else {
                    //connected output --> create new memory address and set it
                    let index = typeIndex2maxOffset.get(outputKV[1].Type)!;
                    typeIndex2globalConnectorIndex2adressOffset.get(outputKV[1].Type)!.set(outputKV[1].GlobalConnectorIndex, index);
                    
                    //add all outgoing links to typeIndex2adressOffset2ListOfLinks
                    typeIndex2adressOffset2ListOfLinks.get(outputKV[1].Type)!.set(index, outputKV[1].GetLinksCopy());
                    index++;
                    typeIndex2maxOffset.set(outputKV[1].Type, index);
                }
            }
        }
        return {
            typeIndex2globalConnectorIndex2adressOffset:typeIndex2globalConnectorIndex2adressOffset,
            typeIndex2adressOffset2ListOfLinks:typeIndex2adressOffset2ListOfLinks,
            typeIndex2maxOffset:typeIndex2maxOffset,
        };
    }


    private serialize(operators: FlowchartOperator[], maps:Maps):HashAndBuf {


        /*
        Lege nun die Operatoren in der durch das Array vorgegebenen Struktur in ein Array ab
        */
        let buffer = new ArrayBuffer(Math.pow(2, 16));
        let serctx = new SerializeContext(buffer, 0);
        let ctx: SerializeContextAndAdressMap = { 
            typeIndex2globalConnectorIndex2adressOffset: maps.typeIndex2globalConnectorIndex2adressOffset, 
            ctx:serctx
        };
        //Version of Data Structure
        serctx.writeU32(0xAFFECAFE);
        //Placeholder for hash, will be overwritten some lines later
        serctx.writeU32(0);


        for (let type in ConnectorType) {
            if (!isNaN(Number(type))) {
                serctx.writeU32(maps.typeIndex2maxOffset.get(Number(type))!);
            }
        }
    
        //operatorsCount
        serctx.writeU32(operators.length);
        for (const operator of operators) {
            operator.SerializeToBinary(ctx);
        }

        let hash= serctx.funhash(8, (2^32)-1);
        serctx.overwriteU32(hash, 4);

        return {
            hash:hash, 
            buf:ctx.ctx.getResult(), 
        };
    }
}