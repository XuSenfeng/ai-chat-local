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

#ifndef _BAIDU_VTT_H_
#define _BAIDU_VTT_H_

#include "esp_err.h"
#include "audio_event_iface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_VTT_BUFFER_SIZE (8192)

/**
 * Baidu Cloud Speech-to-Text audio encoding
 */
typedef enum {
    ENCODING_LINEAR16 = 0,  /*!< Baidu Cloud Speech-to-Text audio encoding PCM 16-bit mono */
} baidu_vtt_encoding_t;

typedef struct baidu_vtt* baidu_vtt_handle_t;
typedef void (*baidu_vtt_event_handle_t)(baidu_vtt_handle_t vtt);

/**
 * baidu Cloud Speech-to-Text configurations
 */
typedef struct {
    int record_sample_rates;            /*!< Audio recording sample rate */
    baidu_vtt_encoding_t encoding;      /*!< Audio encoding */
    int buffer_size;                    /*!< Processing buffer size */
    baidu_vtt_event_handle_t on_begin;  /*!< Begin send audio data to server */
} baidu_vtt_config_t;




/**
 * @brief      initialize Baidu Cloud Speech-to-Text, this function will return a Speech-to-Text context
 *
 * @param      config  The Baidu Cloud Speech-to-Text configuration
 *
 * @return     The Speech-to-Text context
 */
baidu_vtt_handle_t baidu_vtt_init(baidu_vtt_config_t *config);

/**
 * @brief      Start recording and sending audio to Baidu Cloud Speech-to-Text
 *
 * @param[in]  vtt   The Speech-to-Text context
 *
 * @return
 *     - ESP_OK
 *     - ESP_FAIL
 */
esp_err_t baidu_vtt_start(baidu_vtt_handle_t vtt);

/**
 * @brief      Stop sending audio to Baidu Cloud Speech-to-Text and get the result text
 *
 * @param[in]  vtt   The Speech-to-Text context
 *
 * @return     Baidu Cloud Speech-to-Text server response
 */
char *baidu_vtt_stop(baidu_vtt_handle_t vtt);

/**
 * @brief      Cleanup the Speech-to-Text object
 *
 * @param[in]  vtt   The Speech-to-Text context
 *
 * @return
 *  - ESP_OK
 *  - ESP_FAIL
 */
esp_err_t baidu_vtt_destroy(baidu_vtt_handle_t vtt);

/**
 * @brief      Register listener for the Speech-to-Text context
 *
 * @param[in]   vtt   The Speech-to-Text context
 * @param[in]  listener  The listener
 *
 * @return
 *  - ESP_OK
 *  - ESP_FAIL
 */
esp_err_t baidu_vtt_set_listener(baidu_vtt_handle_t vtt, audio_event_iface_handle_t listener);

#ifdef __cplusplus
}
#endif

#endif
