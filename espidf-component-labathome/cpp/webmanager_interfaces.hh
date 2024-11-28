#pragma once
#include "flatbuffers/flatbuffers.h"
#include <esp_err.h>


enum class eMessageReceiverResult{
    OK=0,
    NOT_FOR_ME=1,
    FOR_ME_BUT_FAILED,
};

class MessageSender{
    public:
    virtual esp_err_t WrapAndFinishAndSendAsync(uint32_t namespace, ::flatbuffers::FlatBufferBuilder &_fbb)=0;
};

class iMessageReceiver{
    public:
    virtual eMessageReceiverResult provideWebsocketMessage(MessageSender* callback, httpd_req_t *req, httpd_ws_frame_t *ws_pkt, uint32_t namespace, ::flatbuffers::FlatBufferBuilder &_fbb)=0;
};
