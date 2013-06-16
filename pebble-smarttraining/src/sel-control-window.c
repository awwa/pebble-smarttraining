#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "smarttraining.h"
#include "train-window.h"
#include "sel-train-type-window.h"

/** コントロール選択画面 **/
Window selControlWindow;

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
    SimpleMenuLayer menuLayer;
    /** メニューセクション **/
    SimpleMenuSection menuSections[1];
    /** メニューアイテム **/
    SimpleMenuItem menuItems[2];
    /** アイコン配列 **/
    BmpContainer icons[2];

} sel_control_window_data;

/**
 * 初期化
 */
void sel_control_window_handle_init(AppContextRef ctx) {
    (void) ctx;

    // 画面の初期化
    window_init(&selControlWindow, "SmartTraining");
    window_set_fullscreen(&selControlWindow, true);
    //    window_set_background_color(&selTrainTypeWindow, GColorBlack);

    // メニューレイヤー初期化
    for (int i = 0; i < 2; i++) {
        // 画像コンテナ初期化
        bmp_init_container(IMAGE_RESOURCE_IDS_CONTROL_ICON[i],
                &sel_control_window_data.icons[i]);
        sel_control_window_data.menuItems[i] = (SimpleMenuItem)
        {
            .title = NULL,
            .icon = &sel_control_window_data.icons[i].bmp,
            .callback = sel_control_window_select_item_callback 
        };
    }

    sel_control_window_data.menuSections[0] = (SimpleMenuSection)
    {
        .title = NULL,
        .items = sel_control_window_data.menuItems,
        .num_items = ARRAY_LENGTH(sel_control_window_data.menuItems)
    };
    simple_menu_layer_init(&sel_control_window_data.menuLayer,
            selControlWindow.layer.frame, &selControlWindow,
            sel_control_window_data.menuSections,
            ARRAY_LENGTH(sel_control_window_data.menuSections), NULL);
    layer_add_child(&selControlWindow.layer,
            simple_menu_layer_get_layer(&sel_control_window_data.menuLayer));
}

/**
 * 後始末
 */
void sel_control_window_handle_deinit(AppContextRef ctx) {
    // 画像リソース解放処理
    for (int i = 0; i < 2; i++) {
        bmp_deinit_container(&sel_control_window_data.icons[i]);
    }
}
