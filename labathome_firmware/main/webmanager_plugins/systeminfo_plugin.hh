#pragma once

#include "webmanager_interfaces.hh"
#include "flatbuffers/flatbuffers.h"
#include "../generated/flatbuffers_cpp/systeminfo_generated.h"

class SystemInfoPlugin : public webmanager::iWebmanagerPlugin
{
private:
    esp_err_t sendResponseSystemData(webmanager::iWebmanagerCallback *callback)
    {
        ESP_LOGI(TAG, "Prepare to send ResponseSystemData");
        flatbuffers::FlatBufferBuilder b(1024);
        const esp_partition_t *running = esp_ota_get_running_partition();
        esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, nullptr);
        std::vector<flatbuffers::Offset<systeminfo::PartitionInfo>> partitions_vector;
        while (it)
        {
            const esp_partition_t *p = esp_partition_get(it);
            esp_ota_img_states_t ota_state;
            esp_ota_get_state_partition(p, &ota_state);
            esp_app_desc_t app_info = {};
            esp_ota_get_partition_description(p, &app_info);
            if (app_info.project_name[0] == 0xFF)
            {
                partitions_vector.push_back(systeminfo::CreatePartitionInfoDirect(b, p->label, (uint8_t)p->type, (uint8_t)p->subtype, p->size, (uint8_t)ota_state, p == running, "", "", "", ""));
            }
            else
            {
                partitions_vector.push_back(systeminfo::CreatePartitionInfoDirect(b, p->label, (uint8_t)p->type, (uint8_t)p->subtype, p->size, (uint8_t)ota_state, p == running, app_info.project_name, app_info.version, app_info.date, app_info.time));
            }
            it = esp_partition_next(it);
        }

        esp_chip_info_t chip_info = {};
        esp_chip_info(&chip_info);
        struct timeval tv_now;
        gettimeofday(&tv_now, nullptr);

        float tsens_out{0.0};
        // ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_handle, &tsens_out));
        uint8_t mac_buffer[6];
        esp_read_mac(mac_buffer, ESP_MAC_BT);
        auto bt = systeminfo::Mac6(mac_buffer);
        esp_read_mac(mac_buffer, ESP_MAC_ETH);
        auto eth = systeminfo::Mac6(mac_buffer);
#if CONFIG_SOC_IEEE802154_SUPPORTED
        esp_read_mac(mac_buffer, ESP_MAC_IEEE802154);
#else
        for (int i = 0; i < 6; i++)
            mac_buffer[i] = 0;
#endif
        auto ieee = systeminfo::Mac6(mac_buffer);
        esp_read_mac(mac_buffer, ESP_MAC_WIFI_SOFTAP);
        auto softap = systeminfo::Mac6(mac_buffer);
        esp_read_mac(mac_buffer, ESP_MAC_WIFI_STA);
        auto sta = systeminfo::Mac6(mac_buffer);
        b.Finish(
            systeminfo::CreateResponseWrapper(
            b, 
            systeminfo::Responses::Responses_ResponseSystemData,
            systeminfo::CreateResponseSystemDataDirect(
                b,
                tv_now.tv_sec,
                esp_timer_get_time() / 1000000,
                esp_get_free_heap_size(), 
                &sta, 
                &softap, 
                &bt, 
                &eth, 
                &ieee,
                (uint32_t)chip_info.model,
                chip_info.features,
                chip_info.revision, 
                chip_info.cores, 
                tsens_out, 
                &partitions_vector).Union()
            )
        );
        return callback->WrapAndSendAsync(systeminfo::Namespace::Namespace_Value, b);
    }

public:
    SystemInfoPlugin()
    {
    }

    void OnBegin(webmanager::iWebmanagerCallback *callback) override { (void)(callback); }
    void OnWifiConnect(webmanager::iWebmanagerCallback *callback) override { (void)(callback); }
    void OnWifiDisconnect(webmanager::iWebmanagerCallback *callback) override { (void)(callback); }
    void OnTimeUpdate(webmanager::iWebmanagerCallback *callback) override { (void)(callback); }
    webmanager::eMessageReceiverResult ProvideWebsocketMessage(webmanager::iWebmanagerCallback *callback, httpd_req_t *req, httpd_ws_frame_t *ws_pkt, uint32_t ns, uint8_t *buf) override
    {
        if (ns != systeminfo::Namespace::Namespace_Value)
            return eMessageReceiverResult::NOT_FOR_ME;
        auto rw = flatbuffers::GetRoot<systeminfo::RequestWrapper>(buf);
        auto reqType = rw->request_type();

        switch (reqType)
        {
        case systeminfo::Requests::Requests_RequestRestart:
        {
            esp_restart();
            return webmanager::eMessageReceiverResult::OK;
        }

        case systeminfo::Requests::Requests_RequestSystemData:
        {

            return webmanager::eMessageReceiverResult::OK;
        }
        default:
            return webmanager::eMessageReceiverResult::FOR_ME_BUT_FAILED;
        }
    }
};
