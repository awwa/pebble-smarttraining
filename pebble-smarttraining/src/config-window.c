#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "smarttraining.h"
#include "common.h"

/** 設定画面 **/
Window configWindow;

/** 画面データ **/
static struct ConfigWindowData {
    /** メニューレイヤー **/
    SimpleMenuLayer menuLayer;
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

/**
 * 初期化
 */
void config_window_handle_init(AppContextRef ctx) {
    (void) ctx;

    // 画面の初期化
    window_init(&configWindow, "SmartTraining");
    window_set_fullscreen(&configWindow, true);
    //    window_set_background_color(&selTrainTypeWindow, GColorBlack);

    // メニューレイヤー初期化[0]
    char *buffer = "00.00";
    setVersion(VERSION_MAJOR, VERSION_MINOR, buffer);
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
    simple_menu_layer_init(&config_window_data.menuLayer,
            configWindow.layer.frame, &configWindow,
            config_window_data.menuSections,
            ARRAY_LENGTH(config_window_data.menuSections), NULL);
    layer_add_child(&configWindow.layer,
            simple_menu_layer_get_layer(&config_window_data.menuLayer));
}

/**
 * 後始末
 */
void config_window_handle_deinit(AppContextRef ctx) {
}
