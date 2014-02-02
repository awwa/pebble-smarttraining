#include <pebble.h>

#include "smarttraining.h"
#include "train-window.h"
#include "sel-train-type-window.h"

/** トレーニングタイプ選択画面 **/
Window *selTrainTypeWindow;

/**
 * アイテム選択時処理
 */
void sel_train_type_window_select_item_callback(int index, void *context) {
    
    // TODO PhoneApp側に選択したトレーニングタイプを送信する
    // TODO 仮に直接トレーニングタイプを更新する。後で排除すること。
    //update_training_type_icon(index);
    train_window_select_training_type(index);
    
    // 前の画面に戻る
    window_stack_pop(true);
}

/** 画面データ **/
static struct SelTrainTypeWindowData {
    /** メニューレイヤー **/
    SimpleMenuLayer *menuLayer;
    /** メニューセクション **/
    SimpleMenuSection menuSections[1];
    /** メニューアイテム **/
    SimpleMenuItem menuItems[6];
    /** アイコン配列 **/
    GBitmap *icons[6];

} sel_train_type_window_data;

static void window_load(Window *window) {
    // メニューレイヤー初期化
    for (int i = 0; i < 6; i++) {
        // 画像コンテナ初期化
        sel_train_type_window_data.icons[i] = gbitmap_create_with_resource(IMAGE_RESOURCE_IDS_TRAINING_ICON[i]);
        sel_train_type_window_data.menuItems[i] = (SimpleMenuItem)
        {
            .title = STRING_TRAINING[i],
            .icon = sel_train_type_window_data.icons[i],
            .callback = sel_train_type_window_select_item_callback 
        };
    }

    sel_train_type_window_data.menuSections[0] = (SimpleMenuSection)
    {
        .title = NULL,
        .items = sel_train_type_window_data.menuItems,
        .num_items = ARRAY_LENGTH(sel_train_type_window_data.menuItems)
    };

    // get window frame
    Layer *window_layer = window_get_root_layer(selTrainTypeWindow);
    GRect bounds = layer_get_frame(window_layer);
    sel_train_type_window_data.menuLayer = simple_menu_layer_create( 
            bounds, selTrainTypeWindow,
            sel_train_type_window_data.menuSections,
            ARRAY_LENGTH(sel_train_type_window_data.menuSections), NULL);
    layer_add_child(window_layer,
            simple_menu_layer_get_layer(sel_train_type_window_data.menuLayer));
}

static void window_unload(Window *window) {
    // 画像リソース解放処理
    for (int i = 0; i < 6; i++) {
        gbitmap_destroy(sel_train_type_window_data.icons[i]);
    }
    // メニューの開放処理
    simple_menu_layer_destroy(sel_train_type_window_data.menuLayer);
}

/**
 * 初期化
 */
void sel_train_type_window_handle_init(void) {
    // 画面の初期化
    selTrainTypeWindow = window_create();
    window_set_fullscreen(selTrainTypeWindow, true);
    //    window_set_background_color(&selTrainTypeWindow, GColorBlack);
    window_set_window_handlers(selTrainTypeWindow, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
}

/**
 * 後始末
 */
void sel_train_type_window_handle_deinit(void) {
    window_destroy(selTrainTypeWindow);
}