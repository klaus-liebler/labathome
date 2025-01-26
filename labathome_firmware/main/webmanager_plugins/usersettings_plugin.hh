#pragma once
#include "webmanager_interfaces.hh"
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers_cpp/usersettings_generated.h"

struct SettingCfg
{
    const char *settingKey;
    usersettings::Setting type;
};

class GroupCfg
{
public:
    const char *groupKey;
    size_t setting_len;
    SettingCfg settings[];
};

struct GroupAndStringSetting
{
    const char *groupKey;
    const char *settingkey;
};

struct GroupAndIntegerSetting
{
    const char *groupKey;
    const char *settingkey;
};

struct GroupAndBooleanSetting
{
    const char *groupKey;
    const char *settingkey;
};

struct GroupAndEnumSetting
{
    const char *groupKey;
    const char *settingkey;
};

#include "nvs/nvs_accessor.hh.inc"

class UsersettingsPlugin : public webmanager::iWebmanagerPlugin
{
private:
    const char *partitionName{nullptr};

    const GroupCfg *GetGroup(const char *groupKey)
    {

        for (const GroupCfg *group : groups)
        {
            if (strcmp(groupKey, group->groupKey) == 0)
            {
                return group;
            }
        }
        return nullptr;
    }

public:
    UsersettingsPlugin(const char *partitionName) : partitionName(partitionName) {}

    esp_err_t handleRequestSetUserSettings(const usersettings::RequestSetUserSettings *req, webmanager::iWebmanagerCallback *callback)
    {
        const char *groupKey = req->group_key()->c_str();
        const GroupCfg *group = GetGroup(groupKey);
        RETURN_FAIL_ON_FALSE(group != nullptr, "There is no group with key '%s'", groupKey);
        ESP_LOGI(TAG, "In handleRequestSetUserSettings for GroupKey %s ItemCount %u", groupKey, group->setting_len);

        flatbuffers::FlatBufferBuilder b(1024);
        std::vector<flatbuffers::Offset<flatbuffers::String>> keys_vector;

        nvs_handle_t nvs_handle{0};
        RETURN_ON_ERROR(nvs_open_from_partition(partitionName, group->groupKey, NVS_READWRITE, &nvs_handle));
        ESP_LOGI(TAG, "Successfully opened partition, group: %s with %u items. Updating %lu items", group->groupKey, group->setting_len, req->settings()->size());

        auto settings = req->settings();
        for (size_t i = 0; i < settings->size(); i++)
        {
            auto settingWrapper = settings->Get(i);
            const char *settingKey = settingWrapper->setting_key()->c_str();
            usersettings::Setting setting_type = settingWrapper->setting_type();
            switch (setting_type)
            {
            case usersettings::Setting_IntegerSetting:
            {
                int32_t value = static_cast<const usersettings::IntegerSetting *>(settingWrapper->setting())->value();
                ESP_ERROR_CHECK(nvs_set_i32(nvs_handle, settingKey, value));
                break;
            }
            case usersettings::Setting_EnumSetting:
            {
                int32_t value = static_cast<const usersettings::EnumSetting *>(settingWrapper->setting())->value();
                ESP_ERROR_CHECK(nvs_set_i32(nvs_handle, settingKey, value));
                break;
            }
            case usersettings::Setting_BooleanSetting:
            {
                uint8_t value = static_cast<const usersettings::EnumSetting *>(settingWrapper->setting())->value();
                ESP_ERROR_CHECK(nvs_set_u8(nvs_handle, settingKey, value));
                break;
            }
            case usersettings::Setting_StringSetting:
            {
                const char *value = static_cast<const usersettings::StringSetting *>(settingWrapper->setting())->value()->c_str();
                ESP_ERROR_CHECK(nvs_set_str(nvs_handle, settingKey, value));
                break;
            }
            default:
                break;
            }
            keys_vector.push_back(b.CreateString(settingKey));
        }
        // Close
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
        b.Finish(usersettings::CreateResponseWrapper(
            b,
            usersettings::Responses::Responses_ResponseSetUserSettings,
            usersettings::CreateResponseSetUserSettingsDirect(b, groupKey, &keys_vector).Union()));
        callback->WrapAndSendAsync(usersettings::Namespace::Namespace_Value, b);
        return ESP_OK;
    }

    esp_err_t GetIntegerSetting(const GroupAndIntegerSetting &s, int32_t *value)
    {
        nvs_handle_t nvs_handle{0};
        RETURN_ON_ERROR(nvs_open(s.groupKey, NVS_READONLY, &nvs_handle));
        esp_err_t ret = nvs_get_i32(nvs_handle, s.settingkey, value);
        nvs_close(nvs_handle);
        return ret;
    }

    esp_err_t GetStringSetting(const GroupAndStringSetting &s, char *value, size_t maxLen)
    {
        nvs_handle_t nvs_handle{0};
        RETURN_ON_ERROR(nvs_open(s.groupKey, NVS_READONLY, &nvs_handle));
        esp_err_t ret = nvs_get_str(nvs_handle, s.settingkey, value, &maxLen);
        nvs_close(nvs_handle);
        return ret;
    }

    esp_err_t GetBoolSetting(const GroupAndBooleanSetting &s, bool *value)
    {
        nvs_handle_t nvs_handle{0};
        RETURN_ON_ERROR(nvs_open(s.groupKey, NVS_READONLY, &nvs_handle));
        uint8_t tmp{0};
        esp_err_t ret = nvs_get_u8(nvs_handle, s.settingkey, &tmp);
        nvs_close(nvs_handle);
        *value = tmp != 0;
        return ret;
    }

    esp_err_t GetEnumSetting(const GroupAndEnumSetting &s, int32_t *value)
    {
        nvs_handle_t nvs_handle{0};
        RETURN_ON_ERROR(nvs_open(s.groupKey, NVS_READONLY, &nvs_handle));
        esp_err_t ret = nvs_get_i32(nvs_handle, s.settingkey, value);
        nvs_close(nvs_handle);
        return ret;
    }

    esp_err_t handleRequestGetUserSettings(const usersettings::RequestGetUserSettings *get, webmanager::iWebmanagerCallback *callback)
    {
        const char *groupKey = get->group_key()->c_str();
        const GroupCfg *group = GetGroup(groupKey);
        RETURN_FAIL_ON_FALSE(group != nullptr, "There is no group with key '%s'", groupKey);
        ESP_LOGI(TAG, "In handleRequestGetUserSettings for GroupKey %s ItemCount %u", groupKey, group->setting_len);

        nvs_handle_t nvs_handle{0};
        RETURN_ON_ERROR(nvs_open_from_partition(partitionName, group->groupKey, NVS_READONLY, &nvs_handle));
        ESP_LOGI(TAG, "Successfully opened partition, reading %u items", group->setting_len);
        flatbuffers::FlatBufferBuilder b(1024);
        std::vector<flatbuffers::Offset<usersettings::SettingWrapper>> sw_vector;
        for (size_t i = 0; i < group->setting_len; i++)
        {
            const SettingCfg *settingCfg = &group->settings[i];
            const char *settingKey = settingCfg->settingKey;
            switch (settingCfg->type)
            {
            case usersettings::Setting_IntegerSetting:
            {
                int32_t value{0};
                ESP_ERROR_CHECK(nvs_get_i32(nvs_handle, settingKey, &value));
                sw_vector.push_back(usersettings::CreateSettingWrapperDirect(b, settingKey, usersettings::Setting_IntegerSetting, usersettings::CreateIntegerSetting(b, value).Union()));
                break;
            }
            case usersettings::Setting_EnumSetting:
            {
                int32_t value{0};
                ESP_ERROR_CHECK(nvs_get_i32(nvs_handle, settingKey, &value));
                sw_vector.push_back(usersettings::CreateSettingWrapperDirect(b, settingKey, usersettings::Setting_EnumSetting, usersettings::CreateEnumSetting(b, value).Union()));
                break;
            }
            case usersettings::Setting_BooleanSetting:
            {
                uint8_t value{0};
                ESP_ERROR_CHECK(nvs_get_u8(nvs_handle, settingKey, &value));
                sw_vector.push_back(usersettings::CreateSettingWrapperDirect(b, settingKey, usersettings::Setting_BooleanSetting, usersettings::CreateBooleanSetting(b, value).Union()));
                break;
            }
            case usersettings::Setting_StringSetting:
            {
                /*
                To get the size necessary to store the value, call nvs_get_str with zero out_value and non-zero pointer to length.
                Variable pointed to by length argument will be set to the required length.
                For nvs_get_str, this length includes the zero terminator.
                When calling nvs_get_str with non-zero out_value, length has to be non-zero and has to point to the length available in out_value.
                */

                char *value{nullptr};
                size_t length{0};
                if (nvs_get_str(nvs_handle, settingKey, value, &length) != ESP_OK)
                {
                    ESP_LOGW(TAG, "Can`t sead StringSetting %s", settingKey);
                    break;
                }
                // now, length contains the necessary size
                if (length == 0)
                {
                    ESP_LOGW(TAG, "Read StringSetting %s successfully, but with zero length value", settingKey);
                    value = new char[1]{'\0'};
                }
                else
                {
                    value = new char[length];
                    ESP_ERROR_CHECK(nvs_get_str(nvs_handle, settingKey, value, &length));
                    ESP_LOGI(TAG, "Read StringSetting %s successfully with value %s", settingKey, value);
                }
                auto sw = usersettings::CreateSettingWrapperDirect(b, settingKey, usersettings::Setting_StringSetting, usersettings::CreateStringSettingDirect(b, value).Union());
                delete[] value;
                sw_vector.push_back(sw);
                break;
            }
            default:
                break;
            }
        }
        // Close
        nvs_close(nvs_handle);
        b.Finish(
            usersettings::CreateResponseWrapper(
                b,
                usersettings::Responses::Responses_ResponseGetUserSettings,
                CreateResponseGetUserSettingsDirect(b, groupKey, &sw_vector).Union()));
        callback->WrapAndSendAsync(usersettings::Namespace::Namespace_Value, b);
        return ESP_OK;
    }

    void OnBegin(webmanager::iWebmanagerCallback *callback) override { (void)(callback); }

    void OnWifiConnect(webmanager::iWebmanagerCallback *callback) override { (void)(callback); }
    void OnWifiDisconnect(webmanager::iWebmanagerCallback *callback) override { (void)(callback); }
    void OnTimeUpdate(webmanager::iWebmanagerCallback *callback) override { (void)(callback); }
    webmanager::eMessageReceiverResult ProvideWebsocketMessage(webmanager::iWebmanagerCallback *callback, httpd_req_t *req, httpd_ws_frame_t *ws_pkt, uint32_t ns, uint8_t *buf) override
    {
        if (ns != usersettings::Namespace::Namespace_Value)
            return eMessageReceiverResult::NOT_FOR_ME;
        auto rw = flatbuffers::GetRoot<usersettings::RequestWrapper>(buf);
        switch (rw->request_type())
        {
        case usersettings::Requests::Requests_RequestGetUserSettings:
        {
            handleRequestGetUserSettings(rw->request_as_RequestGetUserSettings(), callback);
            return webmanager::eMessageReceiverResult::OK;
        }
        case usersettings::Requests::Requests_RequestSetUserSettings:
        {
            handleRequestSetUserSettings(rw->request_as_RequestSetUserSettings(), callback);
            return webmanager::eMessageReceiverResult::OK;
        }

        default:
            return webmanager::eMessageReceiverResult::FOR_ME_BUT_FAILED;
        }
    }
};
