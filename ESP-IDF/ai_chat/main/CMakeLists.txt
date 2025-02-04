
file(GLOB_RECURSE DEMO_SOURCES *.c)

set(COMPONENT_SRCS "main.c" "lv_gui.c" "baidu_vtt.c" "minimax_chat.c" "baidu_tts.c" "font_alipuhui20.c" "img_bilibili120.c")
set(COMPONENT_ADD_INCLUDEDIRS .)

register_component()

# Some of warinings from LVGL. Block them.
target_compile_options(${COMPONENT_LIB} PRIVATE -w)

