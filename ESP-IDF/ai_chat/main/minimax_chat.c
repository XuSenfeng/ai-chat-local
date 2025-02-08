/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "sdkconfig.h"
#include "audio_common.h"
#include "audio_hal.h"
#include "minimax_chat.h"
#include "cJSON.h"

static const char *TAG = "MINIMAX_CHAT";

extern const char * minimax_key;
extern nvs_handle my_handle;  
#define PSOT_DATA    "{\
\"messages\":[{\"sender_type\":\"USER\", \"user_id\":%d,\"sender_name\":\"test\",\"text\":\"%s\"}]\
}"

int user_id = -1;
#define MAX_CHAT_BUFFER (2048)
char minimax_content[2048]={0};


char *minimax_chat(const char *text)
{
    char *response_text = NULL;
    char *post_buffer = NULL;
    char *data_buf = NULL; 

    esp_http_client_config_t config = {
        .url = "http://192.168.188.165:8888/",  // 这里替换成自己的GroupId
        .timeout_ms = 40000,
        .buffer_size_tx = 1024  // 默认是512 minimax_key很长 512不够 这里改成1024
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    int post_len = asprintf(&post_buffer, PSOT_DATA, user_id, text); //动态获取一内存记录信息
    
    if (post_buffer == NULL) {
        goto exit_translate;
    }

    // POST
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Authorization", minimax_key);

    if (esp_http_client_open(client, post_len) != ESP_OK) {
        ESP_LOGE(TAG, "Error opening connection");
        goto exit_translate;
    }
    int write_len = esp_http_client_write(client, post_buffer, post_len);
    ESP_LOGI(TAG, "Need to write %d, written %d", post_len, write_len);
    //获取信息长度
    int data_length = esp_http_client_fetch_headers(client);
    if (data_length <= 0) {
        data_length = MAX_CHAT_BUFFER;
    }
    //分配空间
    data_buf = malloc(data_length + 1);
    if(data_buf == NULL) {
        goto exit_translate;
    }
    data_buf[data_length] = '\0';
    int rlen = esp_http_client_read(client, data_buf, data_length);
    data_buf[rlen] = '\0';
    ESP_LOGI(TAG, "read = %s", data_buf);
    //解析信息
    cJSON *root = cJSON_Parse(data_buf);
    char * created = cJSON_GetObjectItem(root,"result")->valuestring;
    int32_t temp_id = cJSON_GetObjectItem(root,"user_id")->valueint;
    if (temp_id != user_id)
    {
        user_id = temp_id;
        ESP_ERROR_CHECK(nvs_set_i32(my_handle, "user_id", user_id));
        ESP_ERROR_CHECK( nvs_commit(my_handle) );
        ESP_LOGI(TAG, "user_id:%d", user_id);
    }
    if(created != 0)
    {
        // ESP_ERR_NVS_INVALID_HANDLE 
        strcpy(minimax_content, created);
        response_text = minimax_content;
        ESP_LOGI(TAG, "response_text:%s", response_text);
    }

    cJSON_Delete(root);

exit_translate:
    free(post_buffer);
    free(data_buf);
    esp_http_client_cleanup(client);

    return response_text;
}