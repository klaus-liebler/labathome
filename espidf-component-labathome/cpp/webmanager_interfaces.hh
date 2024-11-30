#pragma once
#include "flatbuffers/flatbuffers.h"
#include <esp_err.h>
#include <esp_http_server.h>

namespace webmanager
{
    enum class eMessageReceiverResult
    {
        OK = 0,
        NOT_FOR_ME = 1,
        FOR_ME_BUT_FAILED,
    };

    class iWebmanagerCallback
    {
    public:
        virtual esp_err_t WrapAndSendAsync(uint32_t ns, ::flatbuffers::FlatBufferBuilder &_fbb) = 0;
    };

    class iWebmanagerPlugin
    {
    public:
        virtual void OnBegin(iWebmanagerCallback *callback) = 0;
        virtual void OnWifiConnect(iWebmanagerCallback *callback) = 0;
        virtual void OnWifiDisconnect(iWebmanagerCallback *callback) = 0;
        virtual void OnTimeUpdate(iWebmanagerCallback *callback)=0;
        virtual eMessageReceiverResult ProvideWebsocketMessage(iWebmanagerCallback *callback, httpd_req_t *req, httpd_ws_frame_t *ws_pkt, uint32_t ns, uint8_t* buf) = 0;
    };
}