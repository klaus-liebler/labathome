


class NodeWrapperKahn<N>{
  constructor(private id:number, private node:N){}
  get Node(){return this.node}
  get Id(){return this.id}
  public chldId2chldId = new Set<number>();
  public prntId2prntId = new Set<number>();
}



export class TopologicalSortKahn<N> {

  private id2nodes:NodeWrapperKahn<N>[]=[];  
  constructor() {}
  public addNode(id: number, node: N){
    this.id2nodes[id]=new NodeWrapperKahn(id, node);  
    return this;
  }
  addEdge(fromId: number, toId: number) {
      const sourceNode = this.id2nodes[fromId];
      const targetNode = this.id2nodes[toId];
      if(sourceNode && targetNode) {
        sourceNode.chldId2chldId.add(targetNode.Id);
        targetNode.prntId2prntId.add(sourceNode.Id);
      }
      return this;
  }

  public sortKahn(){
    let L: NodeWrapperKahn<N>[] = [];
    let S = new Set<number>();
    this.id2nodes.forEach((n)=>{if(n.prntId2prntId.size==0)S.add(n.Id)});
    while(S.size>0)
    {
      let n_id:number=-1;
      //remove a node n from S
      for(let bar of S) {n_id=bar; break; }
      S.delete(n_id);
      let n = this.id2nodes[n_id];
      //add n to tail of L
      L.push(n);
      //for each node m with an edge e from n to m do
      n.chldId2chldId.forEach(mId => {
        //edge e = from n.Id to m.Id
        //remove edge e from the graph
        n.chldId2chldId.delete(mId);
        let m = this.id2nodes[mId];
        m.prntId2prntId.delete(n.Id);
        if(m.prntId2prntId.size==0) S.add(m.Id);
      });
    }
    return L.map(v=>v.Node);
  }

  private visit(n:NodeWrapperKahn<N>)
  {

  }
}