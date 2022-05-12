#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/dirent.h>

#include "esp_err.h"
#include "esp_log.h"

#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_http_server.h"
#include "http_handlers.hh"
#include "plcmanager.hh"


static const char *TAG = "HTTP_handler";

#include "common_in_project.hh"


constexpr size_t MAX_UPLOAD_FILE_SIZE  = (200*1024); // 200 KB
constexpr const char* MAX_FILE_SIZE_STR="File size must not exceed 200KB!";


esp_err_t helper_get_fbd(httpd_req_t *req, const char *filepath)
{
    
    FILE *fd = NULL;
    struct stat file_stat;
    
    ESP_LOGI(TAG, "Trying to open : %s", filepath);

    if (stat(filepath, &file_stat) == -1) {
        ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
        /* Respond with 404 Not Found */
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
        return ESP_FAIL;
    }

    fd = fopen(filepath, "r");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", filepath, file_stat.st_size);
    httpd_resp_set_type(req, "application/json");

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *chunk = static_cast<char*>(httpd_get_global_user_ctx(req->handle));
    size_t chunksize{0};
    do {
        /* Read file in chunks into the scratch buffer */
        chunksize = fread(chunk, 1, labathome::config::HTTP_SCRATCHPAD_SIZE, fd);

        if (chunksize > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                fclose(fd);
                ESP_LOGE(TAG, "File %s sending failed!", filepath);
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
               return ESP_FAIL;
           }
        }

        /* Keep looping till the whole file is sent */
    } while (chunksize != 0);

    /* Close file after sending complete */
    fclose(fd);
    ESP_LOGI(TAG, "File %s sending complete", filepath);

    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t helper_directory(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Got helper_directory request");
    struct dirent *entry;
    DIR *dir = opendir(labathome::config::paths::FBDSTORE_BASE_DIRECTORY);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr_chunk(req, "[");
    bool firstLoop=true;
    while ((entry = readdir(dir)) != NULL) {
        if(entry->d_type==DT_DIR) continue; //there shound not be a directory, but just in case..
        if(!firstLoop) httpd_resp_sendstr_chunk(req, ",");
        /* Send chunk of HTML file containing table entries with file name and size */
        httpd_resp_sendstr_chunk(req, "\"");
        httpd_resp_sendstr_chunk(req, entry->d_name);
        httpd_resp_sendstr_chunk(req, "\"");
        firstLoop=false;
    }

    closedir(dir);
    httpd_resp_sendstr_chunk(req, "]");
    /* Send empty chunk to signal HTTP response completion */
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}

esp_err_t handle_get_fbdstorejson(httpd_req_t *req)
{
    char filepath[labathome::config::paths::FILE_PATH_MAX];
    const char *filename = strrchr(req->uri, '/')+1;
     if (!filename) {
        ESP_LOGE(TAG, "Filename is not defined correctly");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename is not defined correctly");
        return ESP_FAIL;
    }
    if(strlen(filename)==0)
    {
        /* If name has trailing '/', respond with directory contents */
        return helper_directory(req);
    }
    strcpy(filepath, labathome::config::paths::FBDSTORE_BASE);
    strcpy(filepath+strlen(filepath), filename);
    strcpy(filepath+strlen(filepath), ".json");
    return helper_get_fbd(req, filepath);
}

esp_err_t handle_delete_fbdstorejson(httpd_req_t *req){
    char filepath[labathome::config::paths::FILE_PATH_MAX];
    const char *filename = strrchr(req->uri, '/')+1;
     if (!filename || strlen(filename)==0) {
        ESP_LOGE(TAG, "Filename is not defined correctly");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename is not defined correctly");
        return ESP_FAIL;
    }
    strcpy(filepath, labathome::config::paths::FBDSTORE_BASE);
    strcpy(filepath+strlen(filepath), filename);
    strcpy(filepath+strlen(filepath), ".json");
    struct stat file_stat;
    if (stat(filepath, &file_stat) == -1) {
        ESP_LOGE(TAG, "File does not exist : %s", filename);
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File does not exist");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Deleting file : %s", filename);
    /* Delete file */
    unlink(filepath);
    httpd_resp_sendstr(req, "File deleted successfully");
    return ESP_OK;
}

esp_err_t handle_get_fbddefaultjson(httpd_req_t *req)
{
    return helper_get_fbd(req, labathome::config::paths::DEFAULT_FBD_JSON_FILENAME);
}

esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Error 404: Resource not found");
    return ESP_FAIL;
}


extern const char index_html_gz_start[] asm("_binary_index_html_gz_start");
extern const char index_html_gz_end[] asm("_binary_index_html_gz_end");
extern const size_t index_html_gz_size asm("index_html_gz_length");

esp_err_t handle_get_root(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    httpd_resp_send(req, index_html_gz_start, index_html_gz_size); // -1 = use strlen()
    return ESP_OK;
}


esp_err_t handle_put_fbd(httpd_req_t *req)
{    
    PLCManager *plcmanager = *static_cast<PLCManager **>(req->user_ctx);
    int ret=0;
    int remaining = req->content_len;
    if(remaining>=labathome::config::HTTP_SCRATCHPAD_SIZE)
        return ESP_FAIL;
    ESP_LOGI(TAG, "fbd_put_handler, expecting %d bytes of data", remaining);
    uint8_t *buf= static_cast<uint8_t*>(httpd_get_global_user_ctx(req->handle));
    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, (char*)buf,  MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }
        remaining -= ret;
    }
    // End response
    uint32_t *buf32 = (uint32_t*)buf;
    printf("Received Buffer:\n");
    for(size_t i= 0; i<(req->content_len/4); i++)
    {
        printf("0x%08x ",buf32[i]);
    }
    printf("\n");
    plcmanager->ParseNewExecutableAndEnqueue((uint8_t*)buf, req->content_len);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t handle_get_fbd(httpd_req_t *req){
    PLCManager *plcmanager = *static_cast<PLCManager **>(req->user_ctx);
    size_t size=0;
    plcmanager->GetDebugInfoSize(&size);
    if(size>=labathome::config::HTTP_SCRATCHPAD_SIZE)
        return ESP_FAIL;
    uint8_t *buf= static_cast<uint8_t*>(httpd_get_global_user_ctx(req->handle));
    plcmanager->GetDebugInfo(buf, size);
    httpd_resp_set_type(req, "application/octet-stream");
    httpd_resp_send(req, (char *)buf, size);
    return ESP_OK;
}

esp_err_t handle_get_adcexperiment(httpd_req_t *req)
{
    PLCManager *plcmanager = *static_cast<PLCManager **>(req->user_ctx);
    float *buf;
    plcmanager->GetHAL()->GetADCValues(&buf);
    httpd_resp_set_type(req, "application/octet-stream");
    httpd_resp_send(req, (char *)buf, 4*sizeof(float));
    return ESP_OK;
}

esp_err_t handle_put_fftexperiment(httpd_req_t *req){
    PLCManager *plcmanager = *static_cast<PLCManager **>(req->user_ctx);
    int ret=0;
    int remaining = req->content_len;
    if(remaining!=32){
        ESP_LOGE(TAG, "Unexpected data length %d in handle_put_fft_experiment", remaining);
        return ESP_FAIL;
    }
    uint8_t bufU8[remaining] ALL4;
    while (remaining > 0) {
        if ((ret = httpd_req_recv(req, (char*)bufU8,  MIN(remaining, sizeof(bufU8)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) continue;
            return ESP_FAIL;
        }
        remaining -= ret;
    }
    float *bufF32 = (float*)bufU8;
    //uint32_t *bufU32 = (uint32_t*)bufU8;
    float setpointFan = bufF32[0];

    
    ESP_LOGI(TAG, "Fetching FFT data");
    float magnitudes[64];
    plcmanager->GetHAL()->SetFan1State(setpointFan);
    plcmanager->GetHAL()->SetFan2State(setpointFan);
    plcmanager->GetHAL()->GetFFT64(magnitudes);
    
    httpd_resp_set_type(req, "application/octet-stream");
    httpd_resp_send(req, (const char*)magnitudes, sizeof(magnitudes));
    return ESP_OK;
}


esp_err_t handle_put_heaterexperiment(httpd_req_t *req)
{    
    PLCManager *plcmanager = *static_cast<PLCManager **>(req->user_ctx);
    int ret=0;
    int remaining = req->content_len;
    if(remaining!=24){
        ESP_LOGE(TAG, "Unexpected Data length %d in experiment_put_handler", remaining);
        return ESP_FAIL;
    }
    uint8_t bufU8[remaining] ALL4;
    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, (char*)bufU8,  MIN(remaining, sizeof(bufU8)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }
        remaining -= ret;
    }
    // End response
    float *bufF32 = (float*)bufU8;
    uint32_t *bufU32 = (uint32_t*)bufU8;
    uint32_t modeU32 = bufU32[0];
    float setpointTempOrHeater = bufF32[1];
    float setpointFan = bufF32[2];
    float KP = bufF32[3];
    float TN = bufF32[4];
    float TV = bufF32[5];
    
    ESP_LOGI(TAG, "Set mode %d and setpointTempOrHeater %F and Setpoint Fan %F", modeU32, setpointTempOrHeater, setpointFan);
    HeaterExperimentData returnData;
    switch (modeU32)
    {
    case 0: plcmanager->TriggerHeaterExperimentFunctionblock(&returnData); break;
    case 1: plcmanager->TriggerHeaterExperimentOpenLoop(setpointTempOrHeater, setpointFan, &returnData); break;
    case 2: plcmanager->TriggerHeaterExperimentClosedLoop(setpointTempOrHeater, setpointFan, KP, TN, TV, &returnData); break;
    default:break;
    }
    
    httpd_resp_set_type(req, "application/octet-stream");
    float retbuf[4];
    retbuf[0]=returnData.SetpointTemperature;
    retbuf[1]=returnData.Heater;
    retbuf[2]=returnData.Fan;
    retbuf[3]=returnData.ActualTemperature;
    httpd_resp_send(req, (const char*)retbuf, 16);
    return ESP_OK;
}

esp_err_t handle_put_airspeedexperiment(httpd_req_t *req){
    PLCManager *plcmanager = *static_cast<PLCManager **>(req->user_ctx);
    int ret=0;
    int remaining = req->content_len;
    if(remaining!=24){
        ESP_LOGE(TAG, "Unexpected Data length %d in experiment_put_handler", remaining);
        return ESP_FAIL;
    }
    uint8_t buf[remaining];
    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, (char*)buf,  MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }
        remaining -= ret;
    }
    // End response
    float *bufF32 = (float*)buf;
    uint32_t *bufU32 = (uint32_t*)buf;
    uint32_t modeU32 = bufU32[0];
    float setpointTempOrHeater = bufF32[1];
    float setpointFan = bufF32[2];
    float KP = bufF32[3];
    float TN = bufF32[4];
    float TV = bufF32[5];
    
    ESP_LOGI(TAG, "Set mode %d and setpointTorH %F and Setpoint Fan %F", modeU32, setpointTempOrHeater, setpointFan);
    AirspeedExperimentData returnData;
    switch (modeU32)
    {
    case 0: plcmanager->TriggerAirspeedExperimentFunctionblock(&returnData); break;
    case 1: plcmanager->TriggerAirspeedExperimentOpenLoop(setpointTempOrHeater, setpointFan, &returnData); break;
    case 2: plcmanager->TriggerAirspeedExperimentClosedLoop(setpointTempOrHeater, setpointFan, KP, TN, TV, &returnData); break;
    default:break;
    }
    
    httpd_resp_set_type(req, "application/octet-stream");
    float retbuf[4];
    retbuf[0]=returnData.SetpointAirspeed;
    retbuf[1]=returnData.Fan;
    retbuf[2]=returnData.Servo;
    retbuf[3]=returnData.ActualAirspeed;
    httpd_resp_send(req, (const char*)retbuf, 16);
    return ESP_OK;
}

esp_err_t helper_post_fbd(httpd_req_t *req, const char *filepath, bool overwrite){
    FILE *fd = NULL;
    struct stat file_stat;
    ESP_LOGI(TAG, "Trying to store : %s", filepath);

    if (stat(filepath, &file_stat) == 0) {
        if(overwrite)
        {
            unlink(filepath);
        }
        else
        {
            ESP_LOGE(TAG, "File already exists : %s", filepath);
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File already exists");
            return ESP_FAIL;
        }
    }

    /* File cannot be larger than a limit */
    if (req->content_len > MAX_UPLOAD_FILE_SIZE) {
        ESP_LOGE(TAG, "File too large : %d bytes", req->content_len);
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, MAX_FILE_SIZE_STR);
        /* Return failure to close underlying connection else the
         * incoming file content will keep the socket busy */
        return ESP_FAIL;
    }

    fd = fopen(filepath, "w");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to create file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create file");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Receiving file : %s...", filepath);

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *buf= static_cast<char*>(httpd_get_global_user_ctx(req->handle));
    int received;

    /* Content length of the request gives
     * the size of the file being uploaded */
    int remaining = req->content_len;

    while (remaining > 0) {

        ESP_LOGI(TAG, "Remaining size : %d", remaining);
        /* Receive the file part by part into a buffer */
        if ((received = httpd_req_recv(req, buf, MIN(remaining, labathome::config::HTTP_SCRATCHPAD_SIZE))) <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry if timeout occurred */
                continue;
            }

            /* In case of unrecoverable error,
             * close and delete the unfinished file*/
            fclose(fd);
            unlink(filepath);

            ESP_LOGE(TAG, "File %s reception failed!", filepath);
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
            return ESP_FAIL;
        }

        /* Write buffer content to file on storage */
        if (received && (received != fwrite(buf, 1, received, fd))) {
            /* Couldn't write everything to file!
             * Storage may be full? */
            fclose(fd);
            unlink(filepath);

            ESP_LOGE(TAG, "File %s write failed!", filepath);
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to write file to storage");
            return ESP_FAIL;
        }

        /* Keep track of remaining size of
         * the file left to be uploaded */
        remaining -= received;
    }

    /* Close file upon upload completion */
    fclose(fd);
    ESP_LOGI(TAG, "File reception complete");

    /* Redirect onto root to see the updated file list */
    httpd_resp_sendstr(req, "File uploaded successfully");
    return ESP_OK;
}

esp_err_t handle_post_fbdstorejson(httpd_req_t *req){
    char filepath[labathome::config::paths::FILE_PATH_MAX];
    const char *filename = strrchr(req->uri, '/')+1;
    if (!filename) {
        ESP_LOGE(TAG, "Filename is not defined correctly");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename is not defined correctly");
        return ESP_FAIL;
    }
    strcpy(filepath, labathome::config::paths::FBDSTORE_BASE);
    strcpy(filepath+strlen(filepath), filename);
    strcpy(filepath+strlen(filepath), ".json");
    return helper_post_fbd(req, filepath, false);
}

esp_err_t handle_post_fbddefaultbin(httpd_req_t *req){
    return helper_post_fbd(req, labathome::config::paths::DEFAULT_FBD_BIN_FILENAME, true);
}

esp_err_t handle_post_fbddefaultjson(httpd_req_t *req){
    return helper_post_fbd(req, labathome::config::paths::DEFAULT_FBD_JSON_FILENAME, true);
}
