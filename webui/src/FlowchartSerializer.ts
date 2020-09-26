import { FlowchartOperator } from "./FlowchartOperator";
import {ConnectorType } from "./FlowchartConnector";

export interface SerializeContext {
    typeIndex2globalConnectorIndex2adressOffset: Map<number, Map<number, number>>;
    buffer: DataView;
    bufferOffset: number;

}

declare const msCrypto: Crypto;



export class FlowchartSerializer {

    private static getRandomValuesWithMathRandom(bytes: Uint8Array): void {
        const max = Math.pow(2, 8 * bytes.byteLength / bytes.length);
        for (let i = 0, r; i < bytes.length; i++) {
            bytes[i] = Math.random() * max;
        }
    }


    private static getRandomBytes(length: number): Uint8Array {
        const bytes = new Uint8Array(length);
        if (typeof crypto !== 'undefined') {
            crypto.getRandomValues(bytes);
        } else if (typeof msCrypto !== 'undefined') {
            msCrypto.getRandomValues(bytes);
        } else {
            FlowchartSerializer.getRandomValuesWithMathRandom(bytes);
        }
        return bytes;
    };

    public static Serialize(operators: FlowchartOperator[]):ArrayBuffer {
        let typeIndex2globalConnectorIndex2adressOffset = new Map<number, Map<number, number>>(); //globalConnectorIndex_Outputs 2 variableAdress
        let typeIndex2maxOffset = new Map<number, number>()
        for (let type in ConnectorType) {
            if (!isNaN(Number(type))) {
                typeIndex2globalConnectorIndex2adressOffset.set(Number(type), new Map<number, number>());
                typeIndex2maxOffset.set(Number(type), 2);
            }
        }


        //Iteriere über alle Output-Connectoren aller Operatoren
        //Ein Output, der beschaltet ist, entspricht einer Speicheradresse.
        //Unbeschaltete Outputs schreiben in die Speicheradresse 0. 
        //Unbeschaltete Inputs lesen von der Speicheradresse 1
        //Echte Speicheradressen gibt es dann ab Index 2
        //In den Maps stehen die Zuordnung Globaler Connector Index --> Index der Speicheraddresse
        //Außerdem bekannt: Wie viele Speicheradressen von jedem Typ benötigen wir
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
                    index++;
                    typeIndex2maxOffset.set(outputKV[1].Type, index);
                }
            }
        }

        /*
        Lege nun die Operatoren in der durch das Array vorgegebenen Struktur in ein Array ab
        */
        let buffer = new ArrayBuffer(Math.pow(2, 16));
        let ctx: SerializeContext = { typeIndex2globalConnectorIndex2adressOffset: typeIndex2globalConnectorIndex2adressOffset, buffer: new DataView(buffer), bufferOffset: 0 };
        //Version of Data Structure
        ctx.buffer.setUint32(ctx.bufferOffset, 0xAFFECAFE, true); //Version 0xAFFECAFE means: Development
        ctx.bufferOffset += 4;
        //GUID
        let guid=FlowchartSerializer.getRandomBytes(16)
        guid.forEach((v,i)=>{ctx.buffer.setUint8(ctx.bufferOffset+i, v)}); //guid of the model
        ctx.bufferOffset += 16;


        for (let type in ConnectorType) {
            if (!isNaN(Number(type))) {
                ctx.buffer.setUint32(ctx.bufferOffset, typeIndex2maxOffset.get(Number(type))||2, true);
                ctx.bufferOffset += 4;
            }
        }
        
        
        //operatorsCount
        ctx.buffer.setUint32(ctx.bufferOffset, operators.length, true);
        ctx.bufferOffset += 4;
        for (const operator of operators) {
            operator.SerializeToBinary(ctx);
        }
        let code: String = "const uint8_t code[] = {"
        for (let i = 0; i < ctx.bufferOffset; i++) {
            code += "0x" + ctx.buffer.getUint8(i).toString(16) + ", ";
        }
        code += "};";
        window.alert(code);
        return buffer.slice(0, ctx.bufferOffset)
        
    }

}