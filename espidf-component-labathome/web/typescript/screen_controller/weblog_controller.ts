import { html } from "lit-html";
import { ResponseWrapper, ResponseJournal, RequestJournal, RequestWrapper, Requests, Responses } from "../../generated/flatbuffers/webmanager";
import { MyFavouriteDateTimeFormat } from "../utils/common";
import { ScreenController } from "./screen_controller";
import * as flatbuffers from "flatbuffers"
import { Ref, createRef, ref } from "lit-html/directives/ref.js";

export class WeblogScreenController extends ScreenController {
    onMessage(messageWrapper: ResponseWrapper): void {
        let res = <ResponseJournal>messageWrapper.response(new ResponseJournal());
        this.tblLogs.value!.innerText="";
        for (let i = 0; i < res.journalItemsLength(); i++) {
            let item = res.journalItems(i);
            if(!item)return;
            var row = this.tblLogs.value!.insertRow();
            let secondsEpoch = item.lastMessageTimestamp()!;
            if (secondsEpoch > 1684412222){//this magic second is when I first wrote this code
                row.insertCell().textContent = new Date(1000*Number(secondsEpoch)).toLocaleString("de-DE", MyFavouriteDateTimeFormat);;
            }else{
                row.insertCell().textContent=secondsEpoch.toString();
            }
          
            row.insertCell().textContent = item.messageCode().toString();
            row.insertCell().textContent = item.messageString();
            row.insertCell().textContent = item.messageData().toString();
            row.insertCell().textContent = item.messageCount().toString();
        }
    }

    private tblLogs:Ref<HTMLTableSectionElement> = createRef();

    sendRequestJournal(){
        var b = new flatbuffers.Builder(256);
        this.appManagement.WrapAndFinishAndSend(b,
            Requests.RequestJournal,
            RequestJournal.createRequestJournal(b),
            [Responses.ResponseJournal]
        );
    }

    onCreate(): void {
        this.appManagement.registerWebsocketMessageTypes(this, Responses.ResponseJournal)
        this.sendRequestJournal();
    }

    onFirstStart(): void {
        
    }
    onRestart(): void {
        
    }
    onPause(): void {
        
    }

    public Template =()=> html`<div><input @click=${()=>this.sendRequestJournal()} type="button" value=" ⟳ Update" /></div>
    <table>
        <thead>
            <tr>
                <th>Timestamp</th>
                <th>Code</th>
                <th>Description</th>
                <th>Last Message Data</th>
                <th>Count</th>
            </tr>
        </thead>
        <tbody ${ref(this.tblLogs)}></tbody>
    </table>` 

}
