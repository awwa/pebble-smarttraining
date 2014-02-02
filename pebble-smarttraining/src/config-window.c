#include <pebble.h>

#include "smarttraining.h"
#include "common.h"

/** 設定画面 **/
Window *configWindow;

/** 画面データ **/
static struct ConfigWindowData {
    /** メニューレイヤー **/
    SimpleMenuLayer *menuLayer;
    /** メニューセクション **/
    SimpleMenuSection menuSections[1];
    /** メニューアイテム **/
    SimpleMenuItem menuItems[1];

} config_window_data;

void setVersion(int major, int minor, char* buffer) {
    const char digits[10] = "0123456789";
    if (major > 99) {
        buffer[0] = '9';
        buffer[1] = '9';
    } else if (major > 9) {
        buffer[0] = digits[major / 10];
    } else {
        buffer[0] = '0';
    }
    buffer[1] = digits[major % 10];
    
    if (minor > 99) {
        buffer[3] = '9';
        buffer[4] = '9';
    } else if (minor > 9) {
        buffer[3] = digits[minor / 10];
    } else {
        buffer[3] = '0';
    }
    buffer[3] = digits[minor % 10];
}

static void window_load(Window *window) {
    // メニューレイヤー初期化[0]
    char *buffer = "00.00";
    setVersion(2, 0, buffer);   // TODO jsonを参照すること
    config_window_data.menuItems[0] = (SimpleMenuItem)
    {
        .title = "Version",
        .subtitle = buffer,
        .icon = NULL,
        .callback = NULL 
    };

    config_window_data.menuSections[0] = (SimpleMenuSection)
    {
        .title = NULL,
        .items = config_window_data.menuItems,
        .num_items = ARRAY_LENGTH(config_window_data.menuItems)
    };

    // get window frame
    Layer *window_layer = window_get_root_layer(configWindow);
    GRect bounds = layer_get_frame(window_layer);
    config_window_data.menuLayer = simple_menu_layer_create(
            bounds, configWindow,
            config_window_data.menuSections,
            ARRAY_LENGTH(config_window_data.menuSections), NULL);
    layer_add_child(window_layer,
            simple_menu_layer_get_layer(config_window_data.menuLayer));
}

static void window_unload(Window *window) {
    simple_menu_layer_destroy(config_window_data.menuLayer);
}

/**
 * 初期化
 */
void config_window_handle_init(void) {
    // 画面の初期化
    configWindow = window_create();
    window_set_fullscreen(configWindow, true);
    //    window_set_background_color(&selTrainTypeWindow, GColorBlack);
    window_set_window_handlers(configWindow, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
}

/**
 * 後始末
 */
void config_window_handle_deinit(void) {
    window_destroy(configWindow);
}
