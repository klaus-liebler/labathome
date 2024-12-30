import { IBoardInfo, IApplicationInfo } from "./utils";
import * as esp from "./esp32"
import * as db from "./database"
import * as idf from "./espidf"

export class Context{
  
  private constructor(public b: IBoardInfo, public a: IApplicationInfo, public i: idf.IIdfProjectInfo|null, public f:idf.IFlasherConfiguration|null){}
  private static instance:Context|null;
  public static async get(updateWithCurrentlyConnectedBoard:boolean=false):Promise<Context>{
    if(updateWithCurrentlyConnectedBoard){
      Context.instance=null;
      var esp32 = await esp.GetESP32Object();
      if (!esp32) {
        throw new Error("No connected board found");
      }
      console.log(`Found ${esp32.chipName} on ${esp32.comPort.path} with mac ${esp32.macAsHexString} and encryption key '${esp32.hasEncryptionKey}'`)
      await db.updateDatabase(esp32);
    }
    if(!Context.instance){
      const bi_and_ai=db.getMostRecentlyConnectedBoardInfo();
      const p = idf.GetProjectDescription(bi_and_ai.ai.espIdfProjectDirectory);
      const f = idf.GetFlashArgs(bi_and_ai.ai.espIdfProjectDirectory)
      Context.instance=new Context(bi_and_ai.bi, bi_and_ai.ai, p, f);
    }else{
      if(!Context.instance.i){
        Context.instance.i=idf.GetProjectDescription(Context.instance.a.espIdfProjectDirectory);
      }
      if(!Context.instance.f){
        Context.instance.f = idf.GetFlashArgs(Context.instance.a.espIdfProjectDirectory)
      }
    }
    return Context.instance!;
  }
}