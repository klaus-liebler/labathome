import * as flatbuffers from "flatbuffers"
import { WebSocket } from "ws"
import { Requests, RequestWrapper, Responses, ResponseWrapper } from "./generated/flatbuffers/webmanager";

export function WrapAndFinishAndSend(ws: WebSocket, b:flatbuffers.Builder, message_type:Responses,  message:flatbuffers.Offset){
    console.log(`Send Response ${Requests[message_type]} to client`)
    b.finish(ResponseWrapper.createResponseWrapper(b, message_type, message));
    ws.send(b.asUint8Array());
  }