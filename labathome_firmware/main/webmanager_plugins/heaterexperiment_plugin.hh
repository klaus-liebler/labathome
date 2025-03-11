#pragma once
#include "webmanager_interfaces.hh"
#include "flatbuffers/flatbuffers.h"
#include "../generated/flatbuffers_cpp/ns04heaterexperiment_generated.h"
using namespace webmanager;
class HeaterExperimentPlugin : public webmanager::iWebmanagerPlugin
{
    private:
    DeviceManager* devicemanager;
    public:

    HeaterExperimentPlugin(DeviceManager* devicemanager):devicemanager(devicemanager){

    }
    
    void OnBegin(webmanager::iWebmanagerCallback *callback) override {
        //callback->RegisterNamespace(heaterexperiment::Namespace::Namespace_Value);
    }
    void OnWifiConnect(webmanager::iWebmanagerCallback *callback) override { (void)(callback); }
    void OnWifiDisconnect(webmanager::iWebmanagerCallback *callback) override { (void)(callback); }
    void OnTimeUpdate(webmanager::iWebmanagerCallback *callback) override { (void)(callback); }
    webmanager::eMessageReceiverResult ProvideWebsocketMessage(webmanager::iWebmanagerCallback *callback, httpd_req_t *req, httpd_ws_frame_t *ws_pkt, uint32_t ns, uint8_t *buf) override
    {
        if(ns!=heaterexperiment::Namespace::Namespace_Value){
            return eMessageReceiverResult::NOT_FOR_ME;
        }
        auto rw = flatbuffers::GetRoot<heaterexperiment::RequestWrapper>(buf);
        auto reqType = rw->request_type();
        if(reqType!=heaterexperiment::Requests::Requests_RequestHeater){
            return eMessageReceiverResult::FOR_ME_BUT_FAILED;
        }
        auto r =rw->request_as_RequestHeater();
        ESP_LOGI(TAG, "Set mode %d and setpointTemp %F and setPointHeater %F and fanSpeed %F", r->mode(), r->setpoint_temperature_degrees(), r->heater_power_percent(), r->fan_speed_percent());
        flatbuffers::FlatBufferBuilder b(256);
        devicemanager->TriggerHeaterExperiment(r, b);
        callback->WrapAndSendAsync(heaterexperiment::Namespace::Namespace_Value, b);
        return webmanager::eMessageReceiverResult::OK;
    }
};
