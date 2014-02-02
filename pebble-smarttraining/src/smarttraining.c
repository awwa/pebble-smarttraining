#include <pebble.h>

#include "smarttraining.h"
#include "train-window.h"
#include "sel-train-type-window.h"
#include "sel-control-window.h"
#include "config-window.h"

const int IMAGE_RESOURCE_IDS_TRAINING_ICON[6] = { RESOURCE_ID_WALK,
        RESOURCE_ID_RUN, RESOURCE_ID_SWIM, RESOURCE_ID_WHEELCHAIR,
        RESOURCE_ID_BICYCLE, RESOURCE_ID_CUB };
const char * STRING_TRAINING[6] = { "Walk", "Run", "Swim", "WheelChair",
        "Bicycle", "Cub" };
const int IMAGE_RESOURCE_IDS_CONTROL_ICON[3] = { RESOURCE_ID_STOP,
        RESOURCE_ID_PAUSE, RESOURCE_ID_REC };

/** トレーニング画面 **/
extern Window *trainWindow;

/**
 * 初期化
 */
void handle_init(void) {
    // Tick time開始
    tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
    // PhoneAppとの通信開始
    app_message_open(2048, 2048);
    // トレーニング画面の初期化
    train_window_handle_init();
    // トレーニングタイプ選択画面の初期化
    sel_train_type_window_handle_init();
    // コントロール選択画面の初期化
    sel_control_window_handle_init();
    // 設定画面の初期化
    config_window_handle_init();
}

/**
 * 後始末
 */
void handle_deinit(void) {
    tick_timer_service_unsubscribe();
    // トレーニング画面の後始末
    train_window_handle_deinit();
    // トレーニングタイプ選択画面の後始末
    sel_train_type_window_handle_deinit();
    // コントロール選択画面の後始末
    sel_control_window_handle_deinit();
    // 設定画面の後始末
    config_window_handle_deinit();
}

/**
 * メイン関数
 */
int main(void) {
    // 初期化
    handle_init();
    // イベントループ開始
    app_event_loop();
    // 後始末
    handle_deinit();
    return 0;
}
