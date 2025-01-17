import * as flatbuffers from "flatbuffers"
import * as heat from "../generated/flatbuffers/heaterexperiment"
import { ISender } from "..";
import { randomInt } from "crypto";
export function handleHeaterexperiment(buffer: flatbuffers.ByteBuffer, sender:ISender){
    var req=heat.RequestHeater.getRootAsRequestHeater(buffer)

        let b = new flatbuffers.Builder(1024);
        b.finish(heat.ResponseHeater.createResponseHeater(b, randomInt(40, 60), randomInt(40,60), randomInt(1, 20), 25 ));
        sender.send(heat.Namespace.Value, b);

    
}