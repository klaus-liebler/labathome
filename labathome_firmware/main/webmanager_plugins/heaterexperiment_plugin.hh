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
    webmanager::eMessageReceiverResult ProvideWebsocketMessage(webmanager::iWebmanagerCallback *callback, httpd_req_t *req, httpd_ws_frame_t *ws_pkt, uint16_t namespaceId, uint16_t messageTypeId, const uint8_t *frame, size_t frameLen) override
    {
        if(namespaceId!=WsProtocol::heaterexperiment::NAMESPACE_ID){
            return eMessageReceiverResult::NOT_FOR_ME;
        }
        if(messageTypeId!=WsProtocol::heaterexperiment::RequestHeater::TYPE_ID){
            return eMessageReceiverResult::FOR_ME_BUT_FAILED;
        }
        WsProtocol::heaterexperiment::RequestHeater::Payload r{};
        if(!WsProtocol::heaterexperiment::RequestHeater::Decode(frame, frameLen, r)){
            return eMessageReceiverResult::FOR_ME_BUT_FAILED;
        }
        ESP_LOGI(TAG, "Set mode %d and setpointTemp %F and setPointHeater %F and fanSpeed %F", (int)r.mode, r.setpointTemperatureDegrees, r.heaterPowerPercent, r.fanSpeedPercent);
        uint8_t buf[WsProtocol::heaterexperiment::ResponseHeater::ResponseHeater_MAX_SIZE];
        size_t len{0};
        if(devicemanager->TriggerHeaterExperiment(r, buf, sizeof(buf), &len)!=ErrorCode::OK){
            return eMessageReceiverResult::FOR_ME_BUT_FAILED;
        }
        return callback->SendRawAsync(buf, len)==ESP_OK ? eMessageReceiverResult::OK : eMessageReceiverResult::FOR_ME_BUT_FAILED;
    }
};
#undef TAG
