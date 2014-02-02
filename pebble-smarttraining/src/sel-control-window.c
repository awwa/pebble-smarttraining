#include <pebble.h>

#include "smarttraining.h"
#include "train-window.h"
#include "sel-train-type-window.h"

/** コントロール選択画面 **/
Window *selControlWindow;

/**
 * アイテム選択時処理
 */
void sel_control_window_select_item_callback(int index, void *context) {
    
    // 停止/一時停止を送信する
    if (index == 0) {
        // 停止選択
        train_window_select_control_stop();
    } else {
        // 一時停止選択
        train_window_select_control_pause();
    }
    // 前の画面に戻る
    window_stack_pop(true);
}

/** 画面データ **/
static struct SelControlWindowData {
    /** メニューレイヤー **/
    SimpleMenuLayer *menuLayer;
    /** メニューセクション **/
    SimpleMenuSection menuSections[1];
    /** メニューアイテム **/
    SimpleMenuItem menuItems[2];
    /** アイコン配列 **/
    GBitmap *icons[2];

} sel_control_window_data;

static void window_load(Window *window) {

    // メニューレイヤー初期化
    for (int i = 0; i < 2; i++) {
        // 画像コンテナ初期化
        sel_control_window_data.icons[i] = gbitmap_create_with_resource(IMAGE_RESOURCE_IDS_CONTROL_ICON[i]);
        sel_control_window_data.menuItems[i] = (SimpleMenuItem)
        {
            .title = NULL,
            .icon = sel_control_window_data.icons[i],
            .callback = sel_control_window_select_item_callback 
        };
    }

    sel_control_window_data.menuSections[0] = (SimpleMenuSection)
    {
        .title = NULL,
        .items = sel_control_window_data.menuItems,
        .num_items = ARRAY_LENGTH(sel_control_window_data.menuItems)
    };

    // get window frame
    Layer *window_layer = window_get_root_layer(selControlWindow);
    GRect bounds = layer_get_frame(window_layer);
    sel_control_window_data.menuLayer = simple_menu_layer_create(
            bounds, selControlWindow,
            sel_control_window_data.menuSections,
            ARRAY_LENGTH(sel_control_window_data.menuSections), NULL);
    layer_add_child(window_layer,
            simple_menu_layer_get_layer(sel_control_window_data.menuLayer));

}

static void window_unload(Window *window) {
    // 画像リソース解放処理
    for (int i = 0; i < 2; i++) {
        gbitmap_destroy(sel_control_window_data.icons[i]);
    }
    simple_menu_layer_destroy(sel_control_window_data.menuLayer);
}

/**
 * 初期化
 */
void sel_control_window_handle_init(void) {
    // 画面の初期化
    selControlWindow = window_create();
    window_set_fullscreen(selControlWindow, true);
    //    window_set_background_color(&selTrainTypeWindow, GColorBlack);
    window_set_window_handlers(selControlWindow, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
}

/**
 * 後始末
 */
void sel_control_window_handle_deinit(void) {
    window_destroy(selControlWindow);
}
