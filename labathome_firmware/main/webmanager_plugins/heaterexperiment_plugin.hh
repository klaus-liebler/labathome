#pragma once
#include "webmanager_interfaces.hh"
#include <wsprotocol_cpp/ws_protocol.hh>
#define TAG "HEATER_PLUGIN"
using namespace webmanager;
class HeaterExperimentPlugin : public webmanager::iWebmanagerPlugin
{
    private:
    DeviceManager* devicemanager;
    public:

    HeaterExperimentPlugin(DeviceManager* devicemanager):devicemanager(devicemanager){

    }

    void OnBegin(webmanager::iWebmanagerCallback *callback) override {
    }
    void OnWifiConnect(webmanager::iWebmanagerCallback *callback) override { (void)(callback); }
    void OnWifiDisconnect(webmanager::iWebmanagerCallback *callback) override { (void)(callback); }
    void OnTimeUpdate(webmanager::iWebmanagerCallback *callback) override { (void)(callback); }
    webmanager::eMessageReceiverResult ProvideWebsocketMessage(webmanager::iWebmanagerCallback *callback, WsProtocol::Responder &reply, const uint8_t *frame, size_t frameLen) override
    {
        namespace he = WsProtocol::heaterexperiment;
        if (reply.Request().namespaceId != he::NAMESPACE_ID || reply.Request().messageTypeId != he::RequestHeater::TYPE_ID)
            return eMessageReceiverResult::NOT_FOR_ME;  // another type of heaterexperiment: UnknownMessage
        return Handle(reply, frame, frameLen, he::RequestHeater::Decode, [this](WsProtocol::Responder &rep, const he::RequestHeater::Payload &r) {
            ESP_LOGI(TAG, "Set mode %d and setpointTemp %F and setPointHeater %F and fanSpeed %F", (int)r.mode, r.setpointTemperatureDegrees, r.heaterPowerPercent, r.fanSpeedPercent);
            devicemanager->TriggerHeaterExperiment(r, rep);
            return eMessageReceiverResult::OK;
        });
    }
};
#undef TAG
