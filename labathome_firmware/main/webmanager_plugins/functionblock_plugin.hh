#pragma once

#include "webmanager_interfaces.hh"
#include <wsprotocol_cpp/ws_protocol.hh>
#define TAG "FNCTN_PLUGIN"
using namespace webmanager;
class FunctionblockPlugin : public webmanager::iWebmanagerPlugin
{
    private:
    DeviceManager* devicemanager;

    public:
    FunctionblockPlugin(DeviceManager* devicemanager):devicemanager(devicemanager){

    }

    void OnBegin(webmanager::iWebmanagerCallback *callback) override {

    }
    void OnWifiConnect(webmanager::iWebmanagerCallback *callback) override { (void)(callback); }
    void OnWifiDisconnect(webmanager::iWebmanagerCallback *callback) override { (void)(callback); }
    void OnTimeUpdate(webmanager::iWebmanagerCallback *callback) override { (void)(callback); }
    webmanager::eMessageReceiverResult ProvideWebsocketMessage(webmanager::iWebmanagerCallback *callback, httpd_req_t *req, httpd_ws_frame_t *ws_pkt, uint16_t namespaceId, uint16_t messageTypeId, const uint8_t *frame, size_t frameLen) override
    {
        if(namespaceId!=WsProtocol::functionblock::NAMESPACE_ID) return eMessageReceiverResult::NOT_FOR_ME;

        switch (messageTypeId){
        case WsProtocol::functionblock::RequestDebugData::TYPE_ID:{
            WsProtocol::functionblock::RequestDebugData::Payload r{};
            if(!WsProtocol::functionblock::RequestDebugData::Decode(frame, frameLen, r)) return eMessageReceiverResult::FOR_ME_BUT_FAILED;
            ESP_LOGI(TAG, "Got RequestDebugData");
            static uint8_t buf[WsProtocol::functionblock::ResponseDebugData::ResponseDebugData_MAX_SIZE];
            size_t len{0};
            if(devicemanager->GetDebugInfo(r.requestId, buf, sizeof(buf), &len)!=ErrorCode::OK) return eMessageReceiverResult::FOR_ME_BUT_FAILED;
            return callback->SendRawAsync(buf, len)==ESP_OK ? eMessageReceiverResult::OK : eMessageReceiverResult::FOR_ME_BUT_FAILED;
        }

        case WsProtocol::functionblock::RequestFbdRun::TYPE_ID:
        {
            WsProtocol::functionblock::RequestFbdRun::Payload r{};
            if(!WsProtocol::functionblock::RequestFbdRun::Decode(frame, frameLen, r)) return eMessageReceiverResult::FOR_ME_BUT_FAILED;
            ESP_LOGI(TAG, "Got RequestFbdRun");
            devicemanager->ParseNewExecutableAndEnqueue(TEMPFBD_FBD_FILEPATH);
            WsProtocol::functionblock::ResponseFbdRun::Payload resp{};
            resp.requestId = r.requestId;
            uint8_t buf[16];
            size_t len = WsProtocol::functionblock::ResponseFbdRun::Encode(resp, buf, sizeof(buf));
            return (len>0 && callback->SendRawAsync(buf, len)==ESP_OK) ? eMessageReceiverResult::OK : eMessageReceiverResult::FOR_ME_BUT_FAILED;
        }
        default:
            ESP_LOGW(TAG, "Got Unknown Request");
            break;

        }
        return webmanager::eMessageReceiverResult::FOR_ME_BUT_FAILED;
    }
};
#undef TAG
