import * as flatbuffers from "flatbuffers"
import * as bot from "../generated/flatbuffers/chatbot"
import { ISender } from "..";

export function handleChatbot(buffer: flatbuffers.ByteBuffer, sender: ISender){
    var rw=bot.RequestWrapper.getRootAsRequestWrapper(buffer)
    switch (rw.requestType()) {
        case bot.Requests.RequestChat:{
            const rwc = <bot.RequestChat>rw.request(new bot.RequestChat());
            let b = new flatbuffers.Builder(1024);
            b.finish(bot.ResponseWrapper.createResponseWrapper(b, bot.Responses.ResponseChat, bot.ResponseChat.createResponseChat(b, b.createString("Hello " + rwc.text()))));
            sender.send(bot.Namespace.Value, b);
            
        }
        break;
    }
}