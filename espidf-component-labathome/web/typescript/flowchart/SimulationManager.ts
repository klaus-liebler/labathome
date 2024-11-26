import { SortedOperatorsAndMaps } from "./FlowchartCompiler";
import { FlowchartInputConnector, FlowchartOutputConnector } from "./FlowchartConnector";
import { SimulationContext } from "./SimulationContext";

export class SimulationManager implements SimulationContext{
    private booleans!: boolean[];
    private integers!:number[];
    private floats!:number[];
    private colors!:string[];
    private millisSince1970!:number;
    private running:boolean=false;

    constructor(private sortedOperatorsAndMaps: SortedOperatorsAndMaps){
        this.resetDatastructures()!;
    }

    private resetDatastructures(){
        this.booleans =new Array(this.sortedOperatorsAndMaps.typeIndex2maxOffset.get(0));
        this.integers  = new Array(this.sortedOperatorsAndMaps.typeIndex2maxOffset.get(1));
        this.floats  = new Array(this.sortedOperatorsAndMaps.typeIndex2maxOffset.get(2));
        this.colors  = new Array(this.sortedOperatorsAndMaps.typeIndex2maxOffset.get(3));
        this.millisSince1970=Date.now();
    }


    SetBoolean(outConn: FlowchartOutputConnector, value: boolean): void {
        let i = this.sortedOperatorsAndMaps.typeIndex2globalConnectorIndex2adressOffset.get(0)!.get(outConn.GlobalConnectorIndex)!;
        this.booleans[i]=value;
    }
    SetInteger(outConn: FlowchartOutputConnector, value: number): void {
        let i = this.sortedOperatorsAndMaps.typeIndex2globalConnectorIndex2adressOffset.get(1)!.get(outConn.GlobalConnectorIndex)!;
        this.integers[i]=value;
    }
    SetFloat(outConn: FlowchartOutputConnector, value: number): void {
        let i = this.sortedOperatorsAndMaps.typeIndex2globalConnectorIndex2adressOffset.get(2)!.get(outConn.GlobalConnectorIndex)!;
        this.floats[i]=value;
    }
    SetColor(outConn: FlowchartOutputConnector, value: string): void {
        let i = this.sortedOperatorsAndMaps.typeIndex2globalConnectorIndex2adressOffset.get(3)!.get(outConn.GlobalConnectorIndex)!;
        this.colors[i]=value;
    }
    GetBoolean(inConn: FlowchartInputConnector): boolean {
        
        let i = this.sortedOperatorsAndMaps.typeIndex2globalConnectorIndex2adressOffset.get(0)!.get(inConn.GetGlobalConnectorIndexOfSignalSource())!;
        return this.booleans[i];
    }
    GetInteger(inConn: FlowchartInputConnector): number {
        let i = this.sortedOperatorsAndMaps.typeIndex2globalConnectorIndex2adressOffset.get(1)!.get(inConn.GetGlobalConnectorIndexOfSignalSource())!;
        return this.integers[i];
    }
    GetFloat(inConn: FlowchartInputConnector): number {
        let i = this.sortedOperatorsAndMaps.typeIndex2globalConnectorIndex2adressOffset.get(2)!.get(inConn.GetGlobalConnectorIndexOfSignalSource())!;
        return this.floats[i];
    }
    GetColor(inConn: FlowchartInputConnector): string {
        let i = this.sortedOperatorsAndMaps.typeIndex2globalConnectorIndex2adressOffset.get(3)!.get(inConn.GetGlobalConnectorIndexOfSignalSource())!;
        return this.colors[i];
    }

    public Start(warmstart:boolean=false){
        if(!warmstart) this.resetDatastructures();
        this.running=true;
        for(let o of this.sortedOperatorsAndMaps.sortedOperators){
            o.OnSimulationStart(this);
        }
        window.requestAnimationFrame(()=>this.Step());

    }

    public Stop(){
        this.running=false;
    }


    Step():void{
        this.millisSince1970= Date.now();
        for(let o of this.sortedOperatorsAndMaps.sortedOperators){
            o.OnSimulationStep(this);
        }
        //colorize booleans links
        for(let kv of this.sortedOperatorsAndMaps.typeIndex2adressOffset2ListOfLinks.get(0)!.entries())
        {
            let adressOffset=kv[0];
            if(adressOffset<2) continue;
            let value = this.booleans[adressOffset]
            let linksToChange=kv[1];
            linksToChange.forEach((e)=>{
                e.SetColor(value?"red":"grey");
                e.SetCaption(""+value);
            });
        }

        //colorize integers links
        for(let kv of this.sortedOperatorsAndMaps.typeIndex2adressOffset2ListOfLinks.get(1)!.entries())
        {
            let adressOffset=kv[0];
            if(adressOffset<2) continue;
            let value = this.integers[adressOffset]
            let linksToChange=kv[1];
            linksToChange.forEach((e)=>{
                e.SetCaption(""+value);
            });
        }
        //colorize floats links
        for(let kv of this.sortedOperatorsAndMaps.typeIndex2adressOffset2ListOfLinks.get(2)!.entries())
        {
            let adressOffset=kv[0];
            if(adressOffset<2) continue;
            let value = this.floats[adressOffset]
            let linksToChange=kv[1];
            linksToChange.forEach((e)=>{
                e.SetCaption(""+value);
            });
        }
        //colorize colors links
        for(let kv of this.sortedOperatorsAndMaps.typeIndex2adressOffset2ListOfLinks.get(3)!.entries())
        {
            let adressOffset=kv[0];
            if(adressOffset<2) continue;
            let value = this.colors[adressOffset]
            let linksToChange=kv[1];
            linksToChange.forEach((e)=>{
                e.SetCaption(value);
                e.SetColor(value);
            });
        }
        if(this.running){
            window.requestAnimationFrame(()=>this.Step());
        }
        else{
            for(let o of this.sortedOperatorsAndMaps.sortedOperators){
                o.OnSimulationStop(this);
            }
            for(let types of this.sortedOperatorsAndMaps.typeIndex2adressOffset2ListOfLinks.values())
            {
                for(let kv of types.entries()){
                    let adressOffset=kv[0];
                    if(adressOffset<2) continue;
                    let value = this.booleans[adressOffset]
                    let linksToChange=kv[1];
                    linksToChange.forEach((e)=>{
                        e.SetColor("blue");
                        e.SetCaption("");
                    });
                }
            }
        }
    }

    GetMillis(): number {
        return this.millisSince1970;
    }

}