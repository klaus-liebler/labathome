import * as flatbuffers from "flatbuffers"
import * as wm from "../generated/flatbuffers/webmanager"
import * as weso  from "ws"
import { ResponseWifiConnect, ResponseWifiDisconnect } from "../generated/flatbuffers/webmanager";

const AP_GOOD="Connect to AP -50dB Auth=2";
const AP_BAD="Connect to AP -100dB Auth=2";

export function handleWebmanager(buffer: flatbuffers.ByteBuffer, ws: weso.WebSocket){
    var rw=wm.RequestWrapper.getRootAsRequestWrapper(buffer)
    switch (rw.requestType()) {
        case wm.Requests.RequestWifiConnect:{
            const rwc = <wm.RequestWifiConnect>rw.request(new wm.RequestWifiConnect());
            let b = new flatbuffers.Builder(1024);
            if(rwc.ssid()==AP_GOOD){
                let r = ResponseWifiConnect.createResponseWifiConnect(b, true, b.createString(AP_GOOD), 0xFF101001,0x10101002,0xFF101003, -62);
                b.finish(wm.ResponseWrapper.createResponseWrapper(b, wm.Responses.ResponseWifiConnect, r));
            }else{
                let r = ResponseWifiConnect.createResponseWifiConnect(b, false, b.createString(AP_BAD),0,0,0, -62);
                b.finish(wm.ResponseWrapper.createResponseWrapper(b, wm.Responses.ResponseWifiConnect, r));
            }
            ws.send(b.asUint8Array());
        }
        break;
        case wm.Requests.RequestWifiDisconnect:{
            let b = new flatbuffers.Builder(1024);
            b.finish(wm.ResponseWrapper.createResponseWrapper(b, wm.Responses.ResponseWifiDisconnect, ResponseWifiDisconnect.createResponseWifiDisconnect(b)));
            ws.send(b.asUint8Array());
        }
        break;
        case wm.Requests.RequestNetworkInformation:{
            let b = new flatbuffers.Builder(1024);

            let accesspointsOffset = wm.ResponseNetworkInformation.createAccesspointsVector(b, [
                wm.AccessPoint.createAccessPoint(b, b.createString(AP_BAD), 11, -66, 2),
                wm.AccessPoint.createAccessPoint(b, b.createString(AP_GOOD), 11, -50, 2),
                wm.AccessPoint.createAccessPoint(b, b.createString("AP -76dB Auth=0"), 11, -76, 0),
                wm.AccessPoint.createAccessPoint(b, b.createString("AP -74dB Auth=0"), 11, -74, 0),
                wm.AccessPoint.createAccessPoint(b, b.createString("AP -66dB Auth=0"), 11, -66, 0),
                wm.AccessPoint.createAccessPoint(b, b.createString("AP -59dB Auth=0"), 11, -50, 0)
            ]);
            let r = wm.ResponseNetworkInformation.createResponseNetworkInformation(b, 
                b.createString("MyHostnameKL"), 
                b.createString("MySsidApKL"),  b.createString("Password"), 32,true, b.createString("ssidSta"), 32,43,23,23,accesspointsOffset);
            b.finish(wm.ResponseWrapper.createResponseWrapper(b, wm.Responses.ResponseNetworkInformation, r));
            ws.send(b.asUint8Array());
        }
        break
    }
}