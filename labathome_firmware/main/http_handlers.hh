#pragma once

#include <esp_http_server.h>

esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err);
esp_err_t handle_get_root(httpd_req_t *req);
//Aktuellen einfach nur zur Ausführung bringen
esp_err_t handle_put_fbd(httpd_req_t *req);
//FBD Debuginfo holen
esp_err_t handle_get_fbd(httpd_req_t *req);

esp_err_t handle_get_fbdstorejson(httpd_req_t *req);
esp_err_t handle_get_fbddefaultjson(httpd_req_t *req);

esp_err_t handle_post_fbdstorejson(httpd_req_t *req);
esp_err_t handle_post_fbddefaultbin(httpd_req_t *req);
esp_err_t handle_post_fbddefaultjson(httpd_req_t *req);

esp_err_t handle_delete_fbdstorejson(httpd_req_t *req);

//"Heartbeat" mit Info-Übermittlung fürs Heater-Experiment
esp_err_t handle_put_heaterexperiment(httpd_req_t *req);
esp_err_t handle_put_ptnexperiment(httpd_req_t *req);
esp_err_t handle_put_airspeedexperiment(httpd_req_t *req);
esp_err_t handle_put_fftexperiment(httpd_req_t *req);

