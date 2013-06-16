#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "smarttraining.h"
#include "train-window.h"
#include "sel-train-type-window.h"

/** トレーニングタイプ選択画面 **/
Window selTrainTypeWindow;

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
    SimpleMenuLayer menuLayer;
    /** メニューセクション **/
    SimpleMenuSection menuSections[1];
    /** メニューアイテム **/
    SimpleMenuItem menuItems[6];
    /** アイコン配列 **/
    BmpContainer icons[6];

} sel_train_type_window_data;

/**
 * 初期化
 */
void sel_train_type_window_handle_init(AppContextRef ctx) {
    (void) ctx;

    // 画面の初期化
    window_init(&selTrainTypeWindow, "SmartTraining");
    window_set_fullscreen(&selTrainTypeWindow, true);
    //    window_set_background_color(&selTrainTypeWindow, GColorBlack);

    // メニューレイヤー初期化
    for (int i = 0; i < 6; i++) {
        // 画像コンテナ初期化
        bmp_init_container(IMAGE_RESOURCE_IDS_TRAINING_ICON[i],
                &sel_train_type_window_data.icons[i]);
        sel_train_type_window_data.menuItems[i] = (SimpleMenuItem)
        {
            .title = STRING_TRAINING[i],
            .icon = &sel_train_type_window_data.icons[i].bmp,
            .callback = sel_train_type_window_select_item_callback 
        };
    }

    sel_train_type_window_data.menuSections[0] = (SimpleMenuSection)
    {
        .title = NULL,
        .items = sel_train_type_window_data.menuItems,
        .num_items = ARRAY_LENGTH(sel_train_type_window_data.menuItems)
    };
    simple_menu_layer_init(&sel_train_type_window_data.menuLayer,
            selTrainTypeWindow.layer.frame, &selTrainTypeWindow,
            sel_train_type_window_data.menuSections,
            ARRAY_LENGTH(sel_train_type_window_data.menuSections), NULL);
    layer_add_child(&selTrainTypeWindow.layer,
            simple_menu_layer_get_layer(&sel_train_type_window_data.menuLayer));
}

/**
 * 後始末
 */
void sel_train_type_window_handle_deinit(AppContextRef ctx) {
    // 画像リソース解放処理
    for (int i = 0; i < 6; i++) {
        bmp_deinit_container(&sel_train_type_window_data.icons[i]);
    }
}