#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "smarttraining.h"
#include "train-window.h"
#include "sel-train-type-window.h"
#include "sel-control-window.h"
#include "config-window.h"

#define MY_UUID { 0x0D, 0x63, 0xE9, 0xA4, 0x7D, 0x00, 0x49, 0x89, 0xB1, 0x5C, 0x77, 0xA0, 0x18, 0x8A, 0xC8, 0x59 }
PBL_APP_INFO(MY_UUID,
        "SmartTraining", "awwa",
        VERSION_MAJOR, VERSION_MINOR, /* App version */
        RESOURCE_ID_RUN,
        APP_INFO_STANDARD_APP);

const int IMAGE_RESOURCE_IDS_TRAINING_ICON[6] = { RESOURCE_ID_WALK,
        RESOURCE_ID_RUN, RESOURCE_ID_SWIM, RESOURCE_ID_WHEELCHAIR,
        RESOURCE_ID_BICYCLE, RESOURCE_ID_CUB };
const char * STRING_TRAINING[6] = { "Walk", "Run", "Swim", "WheelChair",
        "Bicycle", "Cub" };
const int IMAGE_RESOURCE_IDS_CONTROL_ICON[3] = { RESOURCE_ID_STOP,
        RESOURCE_ID_PAUSE, RESOURCE_ID_REC };

/** トレーニング画面 **/
extern Window trainWindow;

/**
 * 初期化
 */
void handle_init(AppContextRef ctx) {
    (void) ctx;

    // トレーニング画面の初期化
    train_window_handle_init(ctx);
    // トレーニングタイプ選択画面の初期化
    sel_train_type_window_handle_init(ctx);
    // コントロール選択画面の初期化
    sel_control_window_handle_init(ctx);
    // 設定画面の初期化
    config_window_handle_init(ctx);
}

/**
 * 後始末
 */
void handle_deinit(AppContextRef ctx) {
    (void) ctx;

    // トレーニング画面の後始末
    train_window_handle_deinit(ctx);
    // トレーニングタイプ選択画面の後始末
    sel_train_type_window_handle_deinit(ctx);
    // コントロール選択画面の後始末
    sel_control_window_handle_deinit(ctx);
    // 設定画面の後始末
    config_window_handle_deinit(ctx);
}

/**
 * メイン関数
 */
void pbl_main(void *params) {
    PebbleAppHandlers
    handlers = {
        .init_handler = &handle_init,
        .deinit_handler = &handle_deinit,
        .timer_handler = &handle_timer,
        // メッセージ送受信設定
        .messaging_info = {
            .buffer_sizes = {
                .inbound = 2048,
                .outbound = 2048,
            }
        },
        .tick_info = {
                .tick_handler = &handle_second_tick,
                .tick_units = SECOND_UNIT
        }
    };
    app_event_loop(params, &handlers);
}
