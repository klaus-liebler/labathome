#pragma once

#include "webmanager_interfaces.hh"
#include "flatbuffers/flatbuffers.h"
#include "../generated/flatbuffers_cpp/ns03functionblock_generated.h"
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
    webmanager::eMessageReceiverResult ProvideWebsocketMessage(webmanager::iWebmanagerCallback *callback, httpd_req_t *req, httpd_ws_frame_t *ws_pkt, uint32_t ns, uint8_t *buf) override
    {
        if(ns!=functionblock::Namespace::Namespace_Value) return eMessageReceiverResult::NOT_FOR_ME;
        auto rw = flatbuffers::GetRoot<functionblock::RequestWrapper>(buf);
        auto reqType=rw->request_type();
        
        switch (reqType){
        case functionblock::Requests::Requests_RequestDebugData:{
            ESP_LOGI(TAG, "Got Requests_RequestDebugData");
            size_t debugInfoSize{0};
            devicemanager->GetDebugInfoSize(&debugInfoSize);
            flatbuffers::FlatBufferBuilder b(32+debugInfoSize);
            devicemanager->GetDebugInfo(b);
            callback->WrapAndSendAsync(functionblock::Namespace::Namespace_Value, b);
            return webmanager::eMessageReceiverResult::OK;
        }
        
        case functionblock::Requests::Requests_RequestFbdRun:
        {
            ESP_LOGI(TAG, "Got Requests_RequestFbdRun");
            devicemanager->ParseNewExecutableAndEnqueue(TEMPFBD_FBD_FILEPATH);
            flatbuffers::FlatBufferBuilder b(256);
            b.Finish(
                functionblock::CreateResponseWrapper(
                    b,
                    functionblock::Responses::Responses_ResponseFbdRun,
                    functionblock::CreateResponseFbdRun(b).Union()
                )
            );
            callback->WrapAndSendAsync(functionblock::Namespace::Namespace_Value, b);
            return webmanager::eMessageReceiverResult::OK;
        }
        default:
            ESP_LOGW(TAG, "Got Unknown Request");
            break;

        }
        return webmanager::eMessageReceiverResult::FOR_ME_BUT_FAILED;
    }
};
#undef TAG