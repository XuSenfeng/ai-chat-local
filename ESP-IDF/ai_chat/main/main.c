/* LCD GUI  example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "board.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "audio_idf_version.h"
#include "esp_wifi.h"
#include "esp_http_client.h"
#include "baidu_access_token.h"
#include "esp_peripherals.h"
#include "periph_button.h"
#include "periph_wifi.h"
#include "nvs_flash.h"
#include "baidu_tts.h"
#include "baidu_vtt.h"
#include "minimax_chat.h"
#include "esp_netif.h"
#include <string.h>
#include "esp_timer.h"
#include "lvgl.h"
#include "driver/ledc.h"
#include "lv_gui.h"
#include "esp_lcd_touch_ft5x06.h"

static const char *TAG = "AI_CHAT_EXAMPLE";

char *baidu_access_token = NULL;

#define CONFIG_WIFI_SSID            "jiao"            // �����޸ĳ��Լ���wifi����
#define CONFIG_WIFI_PASSWORD        "11111111"            // �����޸ĳ��Լ���wifi����
#define CONFIG_BAIDU_ACCESS_KEY     "tLfDnkpqLoE0dvjlSjOc1B6y"            // �����޸ĳ��Լ��İٶ�API Key
#define CONFIG_BAIDU_SECRET_KEY     "jh85lHQQ1kaN5HT9AjI10CnOT48YHkMz"            // �����޸ĳ��Լ��İٶ�Secret Key
// �������xxx�滻���Լ���token_key
const char * minimax_key = "Bearer eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJHcm91cE5hbWUiOiLnhKbmtanmtIsiLCJVc2VyTmFtZSI6IueEpua1qea0iyIsIkFjY291bnQiOiIiLCJTdWJqZWN0SUQiOiIxNzkzMjY2NTE2OTU5OTY5Nzk4IiwiUGhvbmUiOiIxODc5MDY3NjMzNSIsIkdyb3VwSUQiOiIxNzkzMjY2NTE2OTUxNTgxMTkwIiwiUGFnZU5hbWUiOiIiLCJNYWlsIjoiIiwiQ3JlYXRlVGltZSI6IjIwMjQtMTAtMjMgMjI6NTk6NTIiLCJpc3MiOiJtaW5pbWF4In0.q1BcT1DpyueDzdm47vSzom6mEU_Y3MwijUpf2jLnIyHP4V3a90_SCiL_WYxefZIsPo4KWrt9BJyTIx4VaS_M3DXB1EdNkfDKu4trkTyHpUSvU8OhvxdWTNtq13O5up5QpRBLKAK-XepQuB6wtsOOE6iPURLKR9URJiIBixC08GSM4p5aT1Kv4l9gRLP_fBWUBfkbwSkTsUU1hSqO3nBDXLJxsElivVbHOy5Sd91jd_MrC3-8zxacKl1HPq8nb4q0rAlTfTC0Z4XJRU4Vh07Zw13aMyrSpuLdd8jm6WB2XGobgLSOm5j6bpOD3DPGMKjtQKwRX7i8M9BW7DIRbJh6tA";


#define LCD_HOST  SPI2_HOST
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (20 * 1000 * 1000)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL  0
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
#define EXAMPLE_PIN_NUM_SCLK           3
#define EXAMPLE_PIN_NUM_MOSI           5
#define EXAMPLE_PIN_NUM_MISO           -1
#define EXAMPLE_PIN_NUM_LCD_DC         6
#define EXAMPLE_PIN_NUM_LCD_RST        -1
#define EXAMPLE_PIN_NUM_LCD_CS         4
#define EXAMPLE_PIN_NUM_BK_LIGHT       2
#define EXAMPLE_PIN_NUM_TOUCH_CS       -1
#define EXAMPLE_LCD_H_RES              320
#define EXAMPLE_LCD_V_RES              240
#define EXAMPLE_LCD_CMD_BITS           8
#define EXAMPLE_LCD_PARAM_BITS         8
#define EXAMPLE_LVGL_TICK_PERIOD_MS    2

int ask_flag = 0;
int answer_flag = 0;
int lcd_clear_flag = 0;

static EventGroupHandle_t my_event_group;
#define ALL_REDAY   BIT0

/**************************** LCD背光调节初�?�化函数 **********************************/
static void lcd_brightness_init(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_13_BIT, // Set duty resolution to 13 bits,
        .freq_hz          = 5000, // Frequency in Hertz. Set frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = EXAMPLE_PIN_NUM_BK_LIGHT,
        .duty           = 0, // Set duty
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

/**************************** LVGL相关处理函数 **********************************/
static bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}

static void example_increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

static void example_lvgl_touch_cb(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
    uint16_t touchpad_x[1] = {0};
    uint16_t touchpad_y[1] = {0};
    uint8_t touchpad_cnt = 0;

    /* Read touch controller data */
    esp_lcd_touch_read_data(drv->user_data);

    /* Get coordinates */
    bool touchpad_pressed = esp_lcd_touch_get_coordinates(drv->user_data, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);

    if (touchpad_pressed && touchpad_cnt > 0) {
        data->point.x = touchpad_x[0];
        data->point.y = touchpad_y[0];
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}


/**************************** LVGL主界面更新函数 **********************************/
extern lv_obj_t *label1;
extern lv_obj_t *label2;
extern char ask_text[256];
extern char minimax_content[2048];

void value_update_cb(lv_timer_t * timer)
{
    if (ask_flag == 1)
    {
        lv_label_set_text_fmt(label1, "我：%s", ask_text);
        ask_flag = 0;
    }
    if (answer_flag == 1)
    {
        lv_label_set_text_fmt(label2, "AI:?%s", minimax_content);
        answer_flag = 0;
    }
    if(lcd_clear_flag == 1)
    {
        lcd_clear_flag = 0;
        lv_label_set_text(label1, "我：");
        lv_label_set_text(label2, "AI:?");
    }
}

/**************************** 主界面任务函数 **********************************/
static void main_page_task(void *pvParameters)
{
    xEventGroupWaitBits(my_event_group, ALL_REDAY, pdFALSE, pdFALSE, portMAX_DELAY);
    lv_obj_clean(lv_scr_act());
    lv_main_page();
    lv_timer_create(value_update_cb, 100, NULL);  // 创建一个lv_timer

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));    
    }

    vTaskDelete(NULL);
}

/**************************** AI对话任务函数 **********************************/
void ai_chat_task(void *pv)
{
    ESP_LOGI(TAG, "[ 1 ] Initialize Buttons & Connect to Wi-Fi network, ssid=%s", CONFIG_WIFI_SSID);
    // Initialize peripherals management
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    //periph_cfg.task_stack = 8*1024;
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);

    periph_wifi_cfg_t wifi_cfg = {
        .wifi_config.sta.ssid = CONFIG_WIFI_SSID,
        .wifi_config.sta.password = CONFIG_WIFI_PASSWORD,
    };
    esp_periph_handle_t wifi_handle = periph_wifi_init(&wifi_cfg);

    // Initialize Button peripheral
    periph_button_cfg_t btn_cfg = {
        .gpio_mask = (1ULL << get_input_mode_id()) | (1ULL << get_input_rec_id()),
    };
    esp_periph_handle_t button_handle = periph_button_init(&btn_cfg);

    // Start wifi & button peripheral
    esp_periph_start(set, button_handle);
    esp_periph_start(set, wifi_handle);

    periph_wifi_wait_for_connected(wifi_handle, portMAX_DELAY);

    // 百度token计算
    if (baidu_access_token == NULL) {
        // Must freed `baidu_access_token` after used
        //这是一个官方的额=示例
        baidu_access_token = baidu_get_access_token(CONFIG_BAIDU_ACCESS_KEY, CONFIG_BAIDU_SECRET_KEY);
    }

    // 百度语音转文字初始化
    baidu_vtt_config_t vtt_config = {
        .record_sample_rates = 16000,
        .encoding = ENCODING_LINEAR16,
    };
    baidu_vtt_handle_t vtt = baidu_vtt_init(&vtt_config);

    // 百度 文字转语音初始化
    baidu_tts_config_t tts_config = {
        .playback_sample_rate = 16000,
    };
    baidu_tts_handle_t tts = baidu_tts_init(&tts_config);

    // 监听事件
    ESP_LOGI(TAG, "[ 4 ] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[4.1] Listening event from the pipeline");
    baidu_vtt_set_listener(vtt, evt);
    baidu_tts_set_listener(tts, evt);

    ESP_LOGI(TAG, "[4.2] Listening event from peripherals");
    audio_event_iface_set_listener(esp_periph_set_get_event_iface(set), evt);

    ESP_LOGI(TAG, "[ 5 ] Listen for all pipeline events");

    // 给出 “已经准备好的信号
    xEventGroupSetBits(my_event_group, ALL_REDAY);

    while (1) {
        audio_event_iface_msg_t msg;
        if (audio_event_iface_listen(evt, &msg, portMAX_DELAY) != ESP_OK) {
            ESP_LOGW(TAG, "[ * ] Event process failed: src_type:%d, source:%p cmd:%d, data:%p, data_len:%d",
                     msg.source_type, msg.source, msg.cmd, msg.data, msg.data_len);
            continue;
        }

        ESP_LOGI(TAG, "[ * ] Event received: src_type:%d, source:%p cmd:%d, data:%p, data_len:%d",
                 msg.source_type, msg.source, msg.cmd, msg.data, msg.data_len);

        if (baidu_tts_check_event_finish(tts, &msg)) {
            ESP_LOGI(TAG, "[ * ] TTS Finish");
            es8311_pa_power(false); // 关闭音�??
            continue;
        }

        if (msg.source_type != PERIPH_ID_BUTTON) {
            continue;
        }

        if ((int)msg.data == get_input_mode_id()) {
            break;
        }

        if ((int)msg.data != get_input_rec_id()) {
            continue;
        }

        if (msg.cmd == PERIPH_BUTTON_PRESSED) {
            //按钮按下
            baidu_tts_stop(tts);
            ESP_LOGI(TAG, "[ * ] Resuming pipeline");
            lcd_clear_flag = 1;
            baidu_vtt_start(vtt);
        } else if (msg.cmd == PERIPH_BUTTON_RELEASE || msg.cmd == PERIPH_BUTTON_LONG_RELEASE) {
            //按钮释放
            ESP_LOGI(TAG, "[ * ] Stop pipeline");

            char *original_text = baidu_vtt_stop(vtt);
            if (original_text == NULL) {
                minimax_content[0]=0; // 清空minimax文本清空
                continue;
            }
            ESP_LOGI(TAG, "Original text = %s", original_text);
            ask_flag = 1;

            char *answer = minimax_chat(original_text);
            if (answer == NULL)
            {
                continue;
            }
            ESP_LOGI(TAG, "minimax answer = %s", answer);
            answer_flag = 1;
            es8311_pa_power(true); // 打开音�??
            baidu_tts_start(tts, answer);
        }

    }
    ESP_LOGI(TAG, "[ 6 ] Stop audio_pipeline");
    baidu_vtt_destroy(vtt);
    baidu_tts_destroy(tts);
    /* Stop all periph before removing the listener */
    esp_periph_set_stop_all(set);
    audio_event_iface_remove_listener(esp_periph_set_get_event_iface(set), evt);

    /* Make sure audio_pipeline_remove_listener & audio_event_iface_remove_listener are called before destroying event_iface */
    audio_event_iface_destroy(evt);
    esp_periph_set_destroy(set);
    vTaskDelete(NULL);
}


/**************************** 主函�? **********************************/
void app_main(void)
{
    /************ 初�?�化NVS *************/
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    ESP_ERROR_CHECK(esp_netif_init());

    /*******  初�?�化ES8311音�?�芯�? (注意：这�?里面会初始化I2C 后面初�?�化触摸屏就不用再初始化I2C�?)  ********/
    ESP_LOGI(TAG, "[ 2 ] Start codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);
    es8311_codec_set_voice_volume(50); // 最大音�?
    es8311_pa_power(false);  // 关闭声音

    /************* 初�?�化液晶�?  ************/
    static lv_disp_draw_buf_t disp_buf; 
    static lv_disp_drv_t disp_drv;      

    ESP_LOGI(TAG, "Initialize SPI bus");
    spi_bus_config_t buscfg = {
        .sclk_io_num = EXAMPLE_PIN_NUM_SCLK,
        .mosi_io_num = EXAMPLE_PIN_NUM_MOSI,
        .miso_io_num = EXAMPLE_PIN_NUM_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = EXAMPLE_LCD_H_RES * 80 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = EXAMPLE_PIN_NUM_LCD_DC,
        .cs_gpio_num = EXAMPLE_PIN_NUM_LCD_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,
        .lcd_param_bits = EXAMPLE_LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = example_notify_lvgl_flush_ready,
        .user_ctx = &disp_drv,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };

    ESP_LOGI(TAG, "Install ST7789 panel driver");
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    /************* 初始化触摸屏 **************/
    esp_lcd_touch_handle_t tp = NULL;
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = EXAMPLE_LCD_V_RES,
        .y_max = EXAMPLE_LCD_H_RES,
        .rst_gpio_num = -1,
        .int_gpio_num = -1,
        .flags = {
            .swap_xy = 1,                                           
            .mirror_x = 1,
            .mirror_y = 0,
        },
    };

    ESP_LOGI(TAG, "Initialize touch controller FT6336");
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, &tp));

    /************ 初始化LVGL *************/
    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();
    
    lv_color_t *buf1 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1);
    lv_color_t *buf2 = heap_caps_malloc(EXAMPLE_LCD_H_RES * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2);
    
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * 20);

    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = EXAMPLE_LCD_H_RES;
    disp_drv.ver_res = EXAMPLE_LCD_V_RES;
    disp_drv.flush_cb = example_lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    ESP_LOGI(TAG, "Install LVGL tick timer");
    
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &example_increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

    static lv_indev_drv_t indev_drv;    // Input device driver (Touch)
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.disp = disp;
    indev_drv.read_cb = example_lvgl_touch_cb;
    indev_drv.user_data = tp;

    lv_indev_drv_register(&indev_drv);

    /******************* 初始化化液晶屏背光 **********************/
    lcd_brightness_init();
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 8191*(1-0.5)));  // 设置占空�? 0.5表示占空比是50%
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0)); // 更新背光

    my_event_group = xEventGroupCreate();

    /****************** 创建任务 **********************/
    lv_gui_start();

    xTaskCreate(main_page_task, "main_page_task", 8192, NULL, 5, NULL);        
    xTaskCreate(ai_chat_task, "ai_chat_task", 8192, NULL, 5, NULL);

    while (1) {
        // raise the task priority of LVGL and/or reduce the handler period can improve the performance
        vTaskDelay(pdMS_TO_TICKS(10));
        // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
        lv_timer_handler();
    }
}
