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
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "audio_hal.h"
#include "http_stream.h"
#include "i2s_stream.h"
#include "mp3_decoder.h"
#include "baidu_vtt.h"
#include "cJSON.h"

static const char *TAG = "BAIDU_VTT";

extern char *baidu_access_token;

#define BAIDU_VTT_ENDPOINT    "http://vop.baidu.com/server_api?dev_pid=1537&cuid=esp32c3&token=%s"
#define BAIDU_VTT_TASK_STACK  (8*1024)


typedef struct baidu_vtt {
    audio_pipeline_handle_t pipeline;
    char                    *buffer;
    audio_element_handle_t  i2s_reader;
    audio_element_handle_t  http_stream_writer;
    int                     sample_rates;
    int                     buffer_size;
    baidu_vtt_encoding_t    encoding;
    char                    *response_text;
    baidu_vtt_event_handle_t on_begin;
} baidu_vtt_t;

char ask_text[256]={0};  // 存储提问的文字

static esp_err_t _http_stream_writer_event_handle(http_stream_event_msg_t *msg)
{
    esp_http_client_handle_t http = (esp_http_client_handle_t)msg->http_client;
    baidu_vtt_t *vtt = (baidu_vtt_t *)msg->user_data;

    static int total_write = 0;
    char len_buf[16];

    if (msg->event_id == HTTP_STREAM_PRE_REQUEST) {
        ESP_LOGI(TAG, "[ + ] HTTP client HTTP_STREAM_PRE_REQUEST, lenght=%d", msg->buffer_len);

        total_write = 0;
        esp_http_client_set_method(http, HTTP_METHOD_POST);
        esp_http_client_set_post_field(http, NULL, -1); // Chunk content
        esp_http_client_set_header(http, "Content-Type", "audio/pcm;rate=16000");
        esp_http_client_set_header(http, "Accept", "application/json");
        return ESP_OK;
    }

    if (msg->event_id == HTTP_STREAM_ON_REQUEST) {  // 这个if会进来好多次
        // write data
        int wlen = sprintf(len_buf, "%x\r\n", msg->buffer_len);
        if (esp_http_client_write(http, len_buf, wlen) <= 0) {
            return ESP_FAIL;
        }
        if (esp_http_client_write(http, msg->buffer, msg->buffer_len) <= 0) {
            return ESP_FAIL;
        }
        if (esp_http_client_write(http, "\r\n", 2) <= 0) {
            return ESP_FAIL;
        }
        total_write += msg->buffer_len;
        printf("\033[A\33[2K\rTotal bytes written: %d\n", total_write);
        return msg->buffer_len;
    }

    /* Write End chunk */
    if (msg->event_id == HTTP_STREAM_POST_REQUEST) {
         ESP_LOGI(TAG, "[ + ] HTTP client HTTP_STREAM_POST_REQUEST, write end chunked marker");
        if (esp_http_client_write(http, "0\r\n\r\n", 5) <= 0) {
            return ESP_FAIL;
        }
        return ESP_OK;
    }

    if (msg->event_id == HTTP_STREAM_FINISH_REQUEST) {
        //结束的事件
        int read_len = esp_http_client_read(http, (char *)vtt->buffer, vtt->buffer_size);
        ESP_LOGI(TAG, "[ + ] HTTP client HTTP_STREAM_FINISH_REQUEST, read_len=%d", read_len);
        if (read_len <= 0) {
            return ESP_FAIL;
        }
        if (read_len > vtt->buffer_size - 1) {
            read_len = vtt->buffer_size - 1;
        }
        vtt->buffer[read_len] = 0; 
        ESP_LOGI(TAG, "Got HTTP Response = %s", (char *)vtt->buffer);
        //参数解析
        cJSON *root = cJSON_Parse(vtt->buffer);
        int err_no = cJSON_GetObjectItem(root,"err_no")->valueint;

        if (err_no == 0) // 如果转换成功
        {
            cJSON *result = cJSON_GetObjectItem(root,"result");
            char *text = cJSON_GetArrayItem(result, 0)->valuestring;
            strcpy(ask_text, text);
            vtt->response_text = ask_text; //记录返回的数据
            ESP_LOGI(TAG, "response_text:%s", vtt->response_text);
        }
        else{
            vtt->response_text = NULL;
        }
        cJSON_Delete(root);

        return ESP_OK;
    }
    return ESP_OK;
}

baidu_vtt_handle_t baidu_vtt_init(baidu_vtt_config_t *config)
{
    // 管道配置
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    baidu_vtt_t *vtt = calloc(1, sizeof(baidu_vtt_t));
    AUDIO_MEM_CHECK(TAG, vtt, return NULL);
    vtt->pipeline = audio_pipeline_init(&pipeline_cfg);

    vtt->buffer_size = config->buffer_size;
    if (vtt->buffer_size <= 0) {
        vtt->buffer_size = DEFAULT_VTT_BUFFER_SIZE;
    }

    vtt->buffer = malloc(vtt->buffer_size);
    AUDIO_MEM_CHECK(TAG, vtt->buffer, goto exit_vtt_init);

    // I2S流配置
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT_WITH_PARA(0, 16000, 16, AUDIO_STREAM_READER);
    i2s_cfg.std_cfg.slot_cfg.slot_mode = I2S_SLOT_MODE_MONO;
    i2s_cfg.std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
    vtt->i2s_reader = i2s_stream_init(&i2s_cfg);

    // HTTP流配置
    http_stream_cfg_t http_cfg = {
        .type = AUDIO_STREAM_WRITER,
        .event_handle = _http_stream_writer_event_handle,
        .user_data = vtt,
        .task_stack = BAIDU_VTT_TASK_STACK,
    };
    vtt->http_stream_writer = http_stream_init(&http_cfg);
    vtt->sample_rates = config->record_sample_rates;
    vtt->encoding = config->encoding;
    vtt->on_begin = config->on_begin;

    // 把 I2S流 和 HTTP流 注册到管道 并链接起来
    audio_pipeline_register(vtt->pipeline, vtt->http_stream_writer, "vtt_http");
    audio_pipeline_register(vtt->pipeline, vtt->i2s_reader,         "vtt_i2s");
    const char *link_tag[2] = {"vtt_i2s", "vtt_http"};
    audio_pipeline_link(vtt->pipeline, &link_tag[0], 2);

    // 设置I2S采样率 位数 等参数
    i2s_stream_set_clk(vtt->i2s_reader, config->record_sample_rates, 16, 1);

    return vtt;
exit_vtt_init:
    baidu_vtt_destroy(vtt);
    return NULL;
}

esp_err_t baidu_vtt_destroy(baidu_vtt_handle_t vtt)
{
    if (vtt == NULL) {
        return ESP_FAIL;
    }
    audio_pipeline_stop(vtt->pipeline);
    audio_pipeline_wait_for_stop(vtt->pipeline);
    audio_pipeline_terminate(vtt->pipeline);
    audio_pipeline_remove_listener(vtt->pipeline);
    audio_pipeline_deinit(vtt->pipeline);
    free(vtt->buffer);
    free(vtt);
    return ESP_OK;
}

esp_err_t baidu_vtt_set_listener(baidu_vtt_handle_t vtt, audio_event_iface_handle_t listener)
{
    if (listener) {
        audio_pipeline_set_listener(vtt->pipeline, listener);
    }
    return ESP_OK;
}

esp_err_t baidu_vtt_start(baidu_vtt_handle_t vtt)
{
    //合成一下百度的信息
    snprintf(vtt->buffer, vtt->buffer_size, BAIDU_VTT_ENDPOINT, baidu_access_token); 
    audio_element_set_uri(vtt->http_stream_writer, vtt->buffer);
    audio_pipeline_reset_items_state(vtt->pipeline);
    audio_pipeline_reset_ringbuffer(vtt->pipeline);
    audio_pipeline_run(vtt->pipeline);
    return ESP_OK;
}


char *baidu_vtt_stop(baidu_vtt_handle_t vtt)
{
    audio_pipeline_stop(vtt->pipeline);
    audio_pipeline_wait_for_stop(vtt->pipeline);

    return vtt->response_text;
}
