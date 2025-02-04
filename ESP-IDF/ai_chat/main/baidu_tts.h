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

#ifndef _BAIDU_TTS_H_
#define _BAIDU_TTS_H_

#include "esp_err.h"
#include "audio_event_iface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_TTS_BUFFER_SIZE (2048)

typedef struct baidu_tts* baidu_tts_handle_t;

typedef struct {
    const char *api_key;
    //const char *lang_code;
    int playback_sample_rate;
    int buffer_size;
} baidu_tts_config_t;

/**
 * @brief      Initialize Baidu Cloud Text-to-Speech, this function will return a Text-to-Speech context
 *
 * @param      config  The configuration
 *
 * @return     The Text-to-Speech context
 */
baidu_tts_handle_t baidu_tts_init(baidu_tts_config_t *config);

/**
 * @brief      Start sending text to Baidu Cloud Text-to-Speech and play audio received
 *
 * @param[in]  tts        The Text-to-Speech context
 * @param[in]  text       The text
 * @param[in]  lang_code  The language code
 *
 * @return
 *  - ESP_OK
 *  - ESP_FAIL
 */
esp_err_t baidu_tts_start(baidu_tts_handle_t tts, const char *text);

/**
 * @brief      Stop playing audio from Baidu Cloud Text-to-Speech
 *
 * @param[in]  tts   The Text-to-Speech context
 *
 * @return
 *  - ESP_OK
 *  - ESP_FAIL
 */
esp_err_t baidu_tts_stop(baidu_tts_handle_t tts);

/**
 * @brief      Register listener for the Text-to-Speech context
 *
 * @param[in]  tts       The Text-to-Speech context
 * @param[in]  listener  The listener
 *
 * @return
 *  - ESP_OK
 *  - ESP_FAIL
 */
esp_err_t baidu_tts_set_listener(baidu_tts_handle_t tts, audio_event_iface_handle_t listener);

/**
 * @brief      Cleanup the Text-to-Speech object
 *
 * @param[in]  tts   The Text-to-Speech context
 *
 * @return
 *  - ESP_OK
 *  - ESP_FAIL
 */
esp_err_t baidu_tts_destroy(baidu_tts_handle_t tts);

/**
 * @brief      Check if the Text-To-Speech finished playing audio from server
 *
 * @param[in]  tts   The Text-to-Speech context
 * @param      msg   The message
 *
 * @return
 *  - true
 *  - false
 */
bool baidu_tts_check_event_finish(baidu_tts_handle_t tts, audio_event_iface_msg_t *msg);

#ifdef __cplusplus
}
#endif

#endif
