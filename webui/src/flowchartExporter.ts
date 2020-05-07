import { FlowchartOperator } from "./flowchartOperator";
import { FlowchartLink } from "./flowchartLink";
import { FlowchartInputConnector, FlowchartOutputConnector } from "./flowchartConnector";
import * as operatorimpl from "./flowchartOperatorImpl";
import {Location2D, Utils} from "./utils"
import "./flowchart.scss";
import { ConnectorType } from "./flowchart";

export class FlowchartExporter {
    public Export(operators:FlowchartOperator[], links:FlowchartLink)
    {
        let booleans= new Map<number, number>();
        let floats= new Map<number, number>();
        let integers= new Map<number, number>();
        let booleanAddr=0;
        let integerAddr=0;
        let floatAddr=0;
        //Ein Output, der beschaltet ist, entspricht einer Speicheradresse. Unbeschaltete Outputs schreiben in die Speicheradresse 0
        for (const operator of operators) {
            for (const outputKV of operator.OutputsKVIt) {
                
                if(outputKV[1].LinksLength==0)
                {
                    //unconnected output
                    switch(outputKV[1].Type){
                        case ConnectorType.BOOLEAN: booleans.set(outputKV[1].Index,0);break;
                        case ConnectorType.INTEGER: integers.set(outputKV[1].Index,0);break;
                        case ConnectorType.FLOAT: floats.set(outputKV[1].Index,0);break;
                    }
                }
                else
                {
                    //connected output --> create new memory address and set it
                    switch(outputKV[1].Type){
                        case ConnectorType.BOOLEAN: booleanAddr++;booleans.set(outputKV[1].Index,booleanAddr);break;
                        case ConnectorType.INTEGER: integerAddr++;integers.set(outputKV[1].Index,integerAddr);break;
                        case ConnectorType.FLOAT: floatAddr++;floats.set(outputKV[1].Index,floatAddr);break;
                    }
                }
                for(const linkKV of outputKV[1].LinksKVIt)
                {
                    //set all others, if availabel
                    switch(outputKV[1].Type){
                        case ConnectorType.BOOLEAN: booleans.set(linkKV[1].To.Index, booleanAddr);break;
                        case ConnectorType.INTEGER: integers.set(linkKV[1].To.Index, integerAddr);break;
                        case ConnectorType.FLOAT: floats.set(linkKV[1].To.Index, floatAddr);break;
                    }
                }
            }
        }
        
        const typedArray1 = new Uint16Array(8);
       
        typedArray1[0] = 32;
    }

}