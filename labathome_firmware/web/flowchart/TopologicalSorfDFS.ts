//https://en.wikipedia.org/wiki/Topological_sorting


enum MarkerState
{
    NONE,
    TEMPORARY,
    PERMANENT,
}

export class  NodeWrapper<N> {
    constructor(private payload:N) {
        this.Mark=MarkerState.NONE;
        this.DependendNodes=[];
    }
    public Mark:MarkerState
    public DependendNodes:NodeWrapper<N>[];
    get Payload() {return this.payload;}
};

export class TopologicalSortDFS<N>{
    private L:Array<NodeWrapper<N>>;
    constructor()
    {
        this.L=new Array<NodeWrapper<N>>();
    }

    public sort(headNode:Array<NodeWrapper<N>>)
    {
        this.L=new Array<NodeWrapper<N>>();
        headNode.forEach((n)=>this.visit(n));
        return this.L;
    }

    private visit(n:NodeWrapper<N>){
        if(n.Mark==MarkerState.PERMANENT) return;
        if(n.Mark==MarkerState.TEMPORARY) throw new Error("not a DAG");
        n.Mark=MarkerState.TEMPORARY;
        for (const d of n.DependendNodes) {
            this.visit(d);
        }
        n.Mark=MarkerState.PERMANENT;
        this.L.push(n);
    }
}