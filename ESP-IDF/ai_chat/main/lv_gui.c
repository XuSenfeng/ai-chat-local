#include "lvgl.h"

lv_obj_t *label1;
lv_obj_t *label2;
lv_obj_t *label_wifi_info;

LV_IMG_DECLARE(img_bilibili120);
LV_FONT_DECLARE(font_alipuhui20);

// 开机界面
void lv_gui_start(void)
{
    // 显示开机GIF图片
    lv_obj_t *gif_start = lv_gif_create(lv_scr_act());
    lv_gif_set_src(gif_start, &img_bilibili120);
    lv_obj_align(gif_start, LV_ALIGN_CENTER, 0, -20);

    // 显示wifi连接信息
    label_wifi_info = lv_label_create(lv_scr_act());
    lv_obj_align(label_wifi_info, LV_ALIGN_BOTTOM_MID, 0, -35);
    lv_obj_set_style_text_font(label_wifi_info, &font_alipuhui20, LV_STATE_DEFAULT);
    lv_label_set_text(label_wifi_info, "正在连接wifi...");
    
}

// 主界面
void lv_main_page(void)
{
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0); // 修改背景为黑色

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_radius(&style, 10);  // 设置圆角半径
    lv_style_set_bg_opa( &style, LV_OPA_COVER );
    lv_style_set_bg_color(&style, lv_color_hex(0x000000));
    lv_style_set_text_color(&style, lv_color_hex(0xffffff)); 
    lv_style_set_border_width(&style, 0);
    lv_style_set_pad_all(&style, 10);
    lv_style_set_width(&style, 320);  // 设置宽
    lv_style_set_height(&style, 240); // 设置高

    /*Create an object with the new style*/
    lv_obj_t * obj = lv_obj_create(lv_scr_act());
    lv_obj_add_style(obj, &style, 0);

    label1 = lv_label_create(obj);
    lv_obj_set_width(label1, 300);
    lv_label_set_long_mode(label1, LV_LABEL_LONG_SCROLL_CIRCULAR);     /*Circular scroll*/
    lv_obj_align(label1, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_font(label1, &font_alipuhui20, LV_STATE_DEFAULT);
    lv_label_set_text(label1, "我：");

    label2 = lv_label_create(obj);
    lv_obj_set_width(label2, 300);
    lv_obj_align(label2, LV_ALIGN_TOP_LEFT, 0, 35);
    lv_obj_set_style_text_font(label2, &font_alipuhui20, LV_STATE_DEFAULT);
    lv_label_set_text(label2, "AI: ");
}