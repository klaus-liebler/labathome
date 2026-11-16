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
    webmanager::eMessageReceiverResult ProvideWebsocketMessage(webmanager::iWebmanagerCallback *callback, WsProtocol::Responder &reply, const uint8_t *frame, size_t frameLen) override
    {
        namespace fb = WsProtocol::functionblock;
        if (reply.Request().namespaceId != fb::NAMESPACE_ID) return eMessageReceiverResult::NOT_FOR_ME;
        switch (reply.Request().messageTypeId) {
        case fb::RequestDebugData::TYPE_ID:
            return Handle(reply, frame, frameLen, fb::RequestDebugData::Decode, [this](WsProtocol::Responder &r, const fb::RequestDebugData::Payload &) {
                ESP_LOGI(TAG, "Got RequestDebugData");
                devicemanager->GetDebugInfo(r);
                return eMessageReceiverResult::OK;
            });
        case fb::RequestFbdRun::TYPE_ID:
            return Handle(reply, frame, frameLen, fb::RequestFbdRun::Decode, [this](WsProtocol::Responder &r, const fb::RequestFbdRun::Payload &) {
                ESP_LOGI(TAG, "Got RequestFbdRun");
                devicemanager->ParseNewExecutableAndEnqueue(TEMPFBD_FBD_FILEPATH);
                r.Send(fb::ResponseFbdRun::Encode, fb::ResponseFbdRun::Payload{});
                return eMessageReceiverResult::OK;
            });
        default:
            ESP_LOGW(TAG, "Got Unknown Request");
            return eMessageReceiverResult::NOT_FOR_ME;  // the requester gets UnknownMessage
        }
    }
};
#undef TAG
