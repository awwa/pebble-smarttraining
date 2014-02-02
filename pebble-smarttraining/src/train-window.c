#include <pebble.h>

#include "smarttraining.h"
#include "train-window.h"
#include "common.h"

#define BITMAP_BUFFER_BYTES 1024

/** トレーニング画面 **/
Window *trainWindow;
/** トレーニングタイプ選択画面 **/
extern Window *selTrainTypeWindow;
/** コントロール選択画面 **/
extern Window *selControlWindow;
/** 設定画面 **/
extern Window *configWindow;

/** 単位系 **/
enum {
    // 単位系：SI
    UNIT_SYSTEM_SI = 0x0,
    // 単位系：USローカル
    UNIT_SYSTEM_US = 0x1,
};

/** 計測状態 **/
enum {
    // 状態：停止中
    STATUS_STOP = 0x0,
    // 状態：計測中
    STATUS_MEASURE = 0x1,
    // 状態：一時停止中
    STATUS_PAUSE = 0x2,
};

/** フォーカスIndex定義 **/
enum {
    TRAIN_WINDOW_FOCUS_UPPER_RIGHT = 0x0,
    TRAIN_WINDOW_FOCUS_MIDDLE = 0x1,
    TRAIN_WINDOW_FOCUS_BOTTOM = 0x2,
    TRAIN_WINDOW_FOCUS_UPPER_LEFT = 0x3,
};

/** 表示中の項目 **/
enum {
    // 継続時間
    ITEM_DURATION = 0x0,
    // 時刻
    ITEM_TIMESTAMP = 0x1,
};

/** PhoneApp<->WatchApp間での情報送信用キー **/
enum {
    // 状態
    KEY_STATUS = 0x00,
    // コマンド
    KEY_CMD = 0x01,
    // バージョン番号送信
    KEY_VER = 0x11,
    // トレーニングタイプ
    KEY_TRAINING_TYPE = 0x02,
    /** データ送信キー：継続時間：値 **/
    KEY_VAL_DURATION = 0x03,
    /** データ送信キー：下段：値 **/
    KEY_VAL_BOTTOM = 0x04,
    /** データ送信キー：下段：単位 **/
    KEY_UNT_BOTTOM = 0x05,
    /** データ送信キー：ストップウォッチ制御 **/
    KEY_CONTROL_STOPWATCH = 0x09,
    /** データ送信キー：バージョン番号取得 **/
    KEY_GET_VERSION = 0x0a,
};

/** コマンド **/
enum {
    // コマンド：距離
    CMD_GET_DISTANCE = 0x01,
    // コマンド：カロリー
    CMD_GET_CALORIES = 0x02,
    // コマンド：速度
    CMD_GET_SPEED = 0x03,
    // コマンド：心拍数
    CMD_GET_HEARTRATE = 0x04,
    // コマンド：ケイデンス
    CMD_GET_CADENCE = 0x05,
    // コマンド：歩数
    CMD_GET_STEPS = 0x06,
    // コマンド：計測開始
    CMD_START = 0x07,
    // コマンド：計測再開
    CMD_RESTART = 0x08,
    // コマンド：計測停止
    CMD_STOP = 0x09,
    // コマンド：一時停止
    CMD_PAUSE = 0x0a,
    /** コマンド：トレーニングタイプ変更：歩く **/
    CMD_SEL_WALK = 0x0b,
    /** コマンド：トレーニングタイプ変更：走る **/
    CMD_SEL_RUN = 0x0c,
    /** コマンド：トレーニングタイプ変更：泳ぐ **/
    CMD_SEL_SWIM = 0x0d,
    /** コマンド：トレーニングタイプ変更：車椅子 **/
    CMD_SEL_WHEELCHAIR = 0x0e,
    /** コマンド：トレーニングタイプ変更：自転車 **/
    CMD_SEL_BICYCLE = 0x0f,
    /** コマンド：トレーニングタイプ変更：カブ **/
    CMD_SEL_CUB = 0x10,
};

enum {
    TYPE_WALK = 0x0,
    TYPE_RUN = 0x1,
    TYPE_SWIM = 0x2,
    TYPE_WHEELCHAIR = 0x3,
    TYPE_BICYCLE = 0x4,
    TYPE_CUB = 0x5,
};

enum {
    CMD_NO_CONTROL = 0x0,
    CMD_CONTROL_START = 0x1,
    CMD_CONTROL_STOP = 0x2,
    CMD_CONTROL_RESTART = 0x3,
};

/** 画面データ **/
static struct TrainWindowData {

    /** アイコン：トレーニングアイコン **/
    BitmapLayer *control_icon_layer;
    GBitmap *control_icon_bitmap;
    uint8_t control_bitmap_data[BITMAP_BUFFER_BYTES];

    /** アイコン：コントロール **/
    BitmapLayer *training_icon_layer;
    GBitmap *training_icon_bitmap;
    uint8_t training_bitmap_data[BITMAP_BUFFER_BYTES];

    /** 中段：継続時間：値 **/
    TextLayer *mdlValLayer;

    /** 下段：値 **/
    TextLayer *btmValLayer;
    /** 下段：単位 **/
    TextLayer *btmUntLayer;
    
    /** 反転レイヤー **/
    InverterLayer *invLayer;
    /** フォーカス：トレーニングアイコン **/
    InverterLayer *focusUpperLeftLayer;
    /** フォーカス：コントロールアイコン **/
    InverterLayer *focusUpperRightLayer;
    /** フォーカス：中段 **/
    InverterLayer *focusMiddleLayer;
    /** フォーカス：下段 **/
    InverterLayer *focusBottomLayer;

    /** フォーカスインデクス **/
    uint32_t indexFocus;

    /** 表示中のデータ：中段 **/
    uint32_t displayItemMiddle;
    /** 表示中のデータ：下段 **/
    uint32_t displayItemBottom;
    /** 単位系 **/
    uint32_t unitSystem;
    /** トレーニングデータ：状態 **/
    uint32_t status;
    /** 選択中のトレーニングタイプ **/
    int trainingType;
    
    /** 表示値：時刻 **/
    char valTimestamp[16];
    /** 表示値：継続時間 **/
    char *valDuration;
    /** 表示値：下段：値 **/
    char valBottom[16];
    /** 表示値：下段：単位 **/
    char untBottom[16];

    /** データ送受信 **/
    AppSync sync;
    uint8_t sync_buffer[2048];

    /** 経過時間 **/
    time_t elapsed_time;
    // 開始時刻
    time_t start_time;
    // 前の時刻情報
    time_t last_pebble_time;
    // ストップウォッチ開始フラグ
    bool started;
    AppTimer *update_timer;

} train_window_data;

/**
 * フォーカス表示更新
 * train_window_data.indexFocusにセットされた値でフォーカスレイヤの表示切替を行う
 */
void update_focus() {
    switch (train_window_data.indexFocus) {
    case TRAIN_WINDOW_FOCUS_UPPER_RIGHT:
        layer_set_hidden(inverter_layer_get_layer(train_window_data.focusUpperRightLayer), false);
        layer_set_hidden(inverter_layer_get_layer(train_window_data.focusBottomLayer), true);
        layer_set_hidden(inverter_layer_get_layer(train_window_data.focusMiddleLayer), true);
        layer_set_hidden(inverter_layer_get_layer(train_window_data.focusUpperLeftLayer), true);
        break;
    case TRAIN_WINDOW_FOCUS_MIDDLE:
        layer_set_hidden(inverter_layer_get_layer(train_window_data.focusUpperRightLayer), true);
        layer_set_hidden(inverter_layer_get_layer(train_window_data.focusBottomLayer), true);
        layer_set_hidden(inverter_layer_get_layer(train_window_data.focusMiddleLayer), false);
        layer_set_hidden(inverter_layer_get_layer(train_window_data.focusUpperLeftLayer), true);
        break;
    case TRAIN_WINDOW_FOCUS_BOTTOM:
        layer_set_hidden(inverter_layer_get_layer(train_window_data.focusUpperRightLayer), true);
        layer_set_hidden(inverter_layer_get_layer(train_window_data.focusBottomLayer), false);
        layer_set_hidden(inverter_layer_get_layer(train_window_data.focusMiddleLayer), true);
        layer_set_hidden(inverter_layer_get_layer(train_window_data.focusUpperLeftLayer), true);
        break;
    case TRAIN_WINDOW_FOCUS_UPPER_LEFT:
        layer_set_hidden(inverter_layer_get_layer(train_window_data.focusUpperRightLayer), true);
        layer_set_hidden(inverter_layer_get_layer(train_window_data.focusBottomLayer), true);
        layer_set_hidden(inverter_layer_get_layer(train_window_data.focusMiddleLayer), true);
        layer_set_hidden(inverter_layer_get_layer(train_window_data.focusUpperLeftLayer), false);
        break;
    default:
        break;
    }
}

/**
 * コマンド送信
 */
static void send_cmd(uint8_t cmd) {
    Tuplet value = TupletInteger(KEY_CMD, cmd);
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    if (iter == NULL)
        return;
    dict_write_tuplet(iter, &value);
    dict_write_end(iter);
    app_message_outbox_send();
}

static void send_ver(int major, int minor) {
    // バージョン番号を計算
    int ver = major * 10 + minor;
    Tuplet value = TupletInteger(KEY_VER, ver);
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    if (iter == NULL)
        return;
    dict_write_tuplet(iter, &value);
    dict_write_end(iter);
    app_message_outbox_send();
}

/**
 * ストップウォッチ初期化
 */
void init_stopwatch() {
    //train_window_data.started = false;  // 初期状態はストップウォッチ停止状態
    train_window_data.elapsed_time = 0;    // 経過時間
    train_window_data.start_time = 0;      // 開始時刻
    train_window_data.last_pebble_time = 0;    // 前の時刻情報
    train_window_data.update_timer = NULL;
}

/**
 * 選択キー押下時イベント
 */
void train_window_select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
    (void) recognizer;

    switch(train_window_data.indexFocus) {
    case TRAIN_WINDOW_FOCUS_UPPER_RIGHT:
        // フォーカス状態：計測制御
        switch (train_window_data.status) {
        case STATUS_MEASURE:
            // 計測中はコントロール選択画面に遷移
            window_stack_push(selControlWindow, true /* Animated */);
            break;
        case STATUS_PAUSE:
            // 一時停止中は計測再開コマンド送信
            send_cmd(CMD_RESTART);
            break;
        case STATUS_STOP:
            // 停止中は計測開始コマンド送信
            init_stopwatch();
            send_cmd(CMD_START);
            break;
        } 
        break;
    case TRAIN_WINDOW_FOCUS_MIDDLE:
        // フォーカス状態：値：中段
        switch (train_window_data.displayItemMiddle) {
        case ITEM_DURATION:
            // 継続時間
            update_disp_item_middle(ITEM_TIMESTAMP);
            break;
        case ITEM_TIMESTAMP:
            // 時刻
            update_disp_item_middle(ITEM_DURATION);
            break;
        }
        break;
    case TRAIN_WINDOW_FOCUS_BOTTOM:
        // フォーカス状態：値：下側
        switch (train_window_data.displayItemBottom) {
        case CMD_GET_DISTANCE:
            // 距離
            switch(train_window_data.trainingType) {
            case TYPE_WALK:
            case TYPE_RUN:
                // 歩く、走る場合のみ歩数表示
                train_window_data.displayItemBottom = CMD_GET_STEPS;
                break;
            case TYPE_SWIM:
            case TYPE_WHEELCHAIR:
            case TYPE_BICYCLE:
            case TYPE_CUB:
                train_window_data.displayItemBottom = CMD_GET_CALORIES;
                break;
            }
            send_cmd(train_window_data.displayItemBottom);
            break;
        case CMD_GET_STEPS:
            // 歩数
            train_window_data.displayItemBottom = CMD_GET_CALORIES;
            send_cmd(train_window_data.displayItemBottom);
            break;
        case CMD_GET_CALORIES:
            // 消費カロリー
            train_window_data.displayItemBottom = CMD_GET_SPEED;
            send_cmd(train_window_data.displayItemBottom);
            break;
        case CMD_GET_SPEED:
            // 速度
            train_window_data.displayItemBottom = CMD_GET_CADENCE;
            send_cmd(train_window_data.displayItemBottom);
            break;
        case CMD_GET_CADENCE:
            // ケイデンス
            train_window_data.displayItemBottom = CMD_GET_HEARTRATE;
            send_cmd(train_window_data.displayItemBottom);
            break;
        case CMD_GET_HEARTRATE:
            // 心拍数
            train_window_data.displayItemBottom = CMD_GET_DISTANCE;
            send_cmd(train_window_data.displayItemBottom);
            break;
        }
        break;
    case TRAIN_WINDOW_FOCUS_UPPER_LEFT:
        // フォーカス状態：トレーニングタイプ
        // トレーニングタイプ選択画面を表示
        switch (train_window_data.status) {
        case STATUS_STOP:
        case STATUS_MEASURE:
            window_stack_push(selTrainTypeWindow, true /* Animated */);
            break;
        }
        break;
    }
}

void train_window_select_long_click_handler(ClickRecognizerRef recognizer, 
        void *context) {
    window_stack_push(configWindow, true);
}

/**
 * 上キー押下時イベント
 */
void train_window_up_single_click_handler(ClickRecognizerRef recognizer, 
        void *context) {
    (void) recognizer;

    // フォーカスの上方向ループ
    switch(train_window_data.indexFocus) {
    case TRAIN_WINDOW_FOCUS_UPPER_RIGHT:
        train_window_data.indexFocus = TRAIN_WINDOW_FOCUS_UPPER_LEFT;
        break;
    case TRAIN_WINDOW_FOCUS_MIDDLE:
        train_window_data.indexFocus = TRAIN_WINDOW_FOCUS_UPPER_RIGHT;
        break;
    case TRAIN_WINDOW_FOCUS_BOTTOM:
        train_window_data.indexFocus = TRAIN_WINDOW_FOCUS_MIDDLE;
        break;
    case TRAIN_WINDOW_FOCUS_UPPER_LEFT:
        train_window_data.indexFocus = TRAIN_WINDOW_FOCUS_BOTTOM;
        break;
    }

    // フォーカス更新
    update_focus();
}

/**
 * 下キー押下時イベント
 */
void train_window_down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
    (void) recognizer;

    // フォーカスの下方向ループ
    switch(train_window_data.indexFocus) {
    case TRAIN_WINDOW_FOCUS_UPPER_RIGHT:
        train_window_data.indexFocus = TRAIN_WINDOW_FOCUS_MIDDLE;
        break;
    case TRAIN_WINDOW_FOCUS_MIDDLE:
        train_window_data.indexFocus = TRAIN_WINDOW_FOCUS_BOTTOM;
        break;
    case TRAIN_WINDOW_FOCUS_BOTTOM:
        train_window_data.indexFocus = TRAIN_WINDOW_FOCUS_UPPER_LEFT;
        break;
    case TRAIN_WINDOW_FOCUS_UPPER_LEFT:
        train_window_data.indexFocus = TRAIN_WINDOW_FOCUS_UPPER_RIGHT;
        break;
    }
    // フォーカス更新
    update_focus();
}

/**
 * クリックイベント登録
 */
void train_window_click_config_provider(void *context) {

    window_set_click_context(BUTTON_ID_UP, context);

    window_single_click_subscribe(BUTTON_ID_SELECT, 
            train_window_select_single_click_handler);
    window_long_click_subscribe(BUTTON_ID_SELECT,
            1000, train_window_select_long_click_handler, NULL);

    window_single_repeating_click_subscribe(BUTTON_ID_UP,
            100, train_window_up_single_click_handler);
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN,
            100, train_window_down_single_click_handler);
}

/**
 * アイコンデータ生成処理
 */
static void mkbitmap(GBitmap* bitmap, const uint8_t* data) {
  bitmap->addr = (void*)data + 12;
  bitmap->row_size_bytes = ((uint16_t*)data)[0];
  bitmap->info_flags = ((uint16_t*)data)[1];
  bitmap->bounds.origin.x = 0;
  bitmap->bounds.origin.y = 0;
  bitmap->bounds.size.w = ((int16_t*)data)[4];
  bitmap->bounds.size.h = ((int16_t*)data)[5];
}

/**
 * 指定したリソースIDのアイコンデータをコントロールアイコンとして読み込む
 */
void load_control_bitmap(uint32_t resource_id) {
    train_window_data.control_icon_bitmap = gbitmap_create_with_resource(resource_id);
    bitmap_layer_set_bitmap(train_window_data.control_icon_layer, train_window_data.control_icon_bitmap);
}

/**
 * 指定したリソースIDのアイコンデータをトレーニングアイコンとして読み込む
 */
void load_training_bitmap(uint32_t resource_id) {
    train_window_data.training_icon_bitmap = gbitmap_create_with_resource(resource_id);
    bitmap_layer_set_bitmap(train_window_data.training_icon_layer, train_window_data.training_icon_bitmap);
}

/**
 * 状態に応じてコントロールアイコンの更新
 */
void update_status_icon(int status) {

    // 計測状態に応じてタイマーの制御
    if (status == STATUS_MEASURE && 
        train_window_data.status != STATUS_MEASURE) {
        // 計測中はタイマー開始
        start_stopwatch();
    }
    if (status != STATUS_MEASURE) {
        // 停止中はタイマー停止
        stop_stopwatch();
    }

    train_window_data.status = status;
    switch (status) {
    case STATUS_STOP:
        // 停止中は計測開始可能なアイコン表示
        load_control_bitmap(IMAGE_RESOURCE_IDS_CONTROL_ICON[2]);
        layer_mark_dirty(bitmap_layer_get_layer(train_window_data.control_icon_layer));
        break;
    case STATUS_PAUSE:
        // 一時停止中は計測開始可能なアイコン表示
        load_control_bitmap(IMAGE_RESOURCE_IDS_CONTROL_ICON[2]);
        layer_mark_dirty(bitmap_layer_get_layer(train_window_data.control_icon_layer));
        break;
    case STATUS_MEASURE:
        // 計測中は停止可能なアイコン表示
        load_control_bitmap(IMAGE_RESOURCE_IDS_CONTROL_ICON[0]);
        layer_mark_dirty(bitmap_layer_get_layer(train_window_data.control_icon_layer));
        break;
    }
}

/**
 * トレーニングタイプアイコンの更新
 * train_window_data.trainingTypeにセットされている値に応じたアイコンに更新する
 */
void update_training_type_icon(int trainingType) {
    train_window_data.trainingType = trainingType;
    load_training_bitmap(IMAGE_RESOURCE_IDS_TRAINING_ICON[train_window_data.trainingType]);
    layer_mark_dirty(bitmap_layer_get_layer(train_window_data.training_icon_layer));
}

/**
 * 中段の表示更新
 */
void update_disp_item_middle(uint32_t displayItem) {
    train_window_data.displayItemMiddle = displayItem;
    switch (displayItem) {
    case ITEM_DURATION:
        // 継続時間
        text_layer_set_text(train_window_data.mdlValLayer, train_window_data.valDuration);
        break;
    case ITEM_TIMESTAMP:
        // 時刻
        text_layer_set_text(train_window_data.mdlValLayer, train_window_data.valTimestamp);
        break;
    }
}

/**
 * ストップウォッチの開始
 */
void start_stopwatch() {
    train_window_data.started = true;
    train_window_data.last_pebble_time = 0;
    train_window_data.start_time = 0;
    train_window_data.update_timer = app_timer_register(100, handle_timer, NULL);
}

/**
 * ストップウォッチの停止
 */
void stop_stopwatch() {
    train_window_data.started = false;
    if (train_window_data.update_timer != NULL) {
        app_timer_cancel(train_window_data.update_timer);
        train_window_data.update_timer = NULL;
    }
}

/**
 * ストップウォッチの再開
 */
void restart_stopwatch() {
    train_window_data.started = true;
    train_window_data.update_timer = app_timer_register(100, handle_timer, NULL);
}

// TODO: Error handling
/**
 * データ受信エラーコールバック
 */
static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  (void) dict_error;
  (void) app_message_error;
  (void) context;
}

/**
 * データ受信コールバック
 */
static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tupel, void* context) {
    switch(key) {
    case KEY_STATUS:
        // 計測状態
        update_status_icon(new_tuple->value->int8);
        break;
    case KEY_TRAINING_TYPE:
        // トレーニングタイプ
        update_training_type_icon(new_tuple->value->int8);
        break;
    case KEY_VAL_DURATION:
        train_window_data.elapsed_time = new_tuple->value->int32 * 1000;
        if (train_window_data.displayItemMiddle == ITEM_DURATION)
            update_duration();
        break;
    case KEY_VAL_BOTTOM:
        strncpy(train_window_data.valBottom, new_tuple->value->cstring, 16);
        layer_mark_dirty(text_layer_get_layer(train_window_data.btmValLayer));
        break;
    case KEY_UNT_BOTTOM:
        strncpy(train_window_data.untBottom, new_tuple->value->cstring, 16);
        layer_mark_dirty(text_layer_get_layer(train_window_data.btmUntLayer));
        break;
    case KEY_CONTROL_STOPWATCH:
        switch ( new_tuple->value->int8 ) {
        case CMD_NO_CONTROL:
            break;
        case CMD_CONTROL_START:
            vibes_short_pulse();
            start_stopwatch();
            break;
        case CMD_CONTROL_STOP:
            vibes_short_pulse();
            stop_stopwatch();
            break;
        case CMD_CONTROL_RESTART:
            vibes_short_pulse();
            restart_stopwatch();
            break;
        }
        break;
    case KEY_GET_VERSION:
        // バージョン番号取得コマンド受信したらバージョン番号返す
        if (new_tuple->value->int8 > 0)
            send_ver(2, 0); // TODO jsonを参照すること
        break;
    }
}

/**
 * コントロール選択画面で一時停止が選択された
 */
void train_window_select_control_pause() {
    // 一時停止コマンド送信
    send_cmd(CMD_PAUSE);
}

/**
 * コントロール選択画面で停止が選択された
 */
void train_window_select_control_stop() {
    // 停止コマンド送信
    send_cmd(CMD_STOP);
}

/**
 * トレーニングタイプ選択画面でトレーニングタイプが選択された
 */
void train_window_select_training_type(int index) {
    switch (index) {
    case TYPE_WALK:
        send_cmd(CMD_SEL_WALK);
        break;
    case TYPE_RUN:
        send_cmd(CMD_SEL_RUN);
        break;
    case TYPE_SWIM:
        send_cmd(CMD_SEL_SWIM);
        break;
    case TYPE_WHEELCHAIR:
        send_cmd(CMD_SEL_WHEELCHAIR);
        break;
    case TYPE_BICYCLE:
        send_cmd(CMD_SEL_BICYCLE);
        break;
    case TYPE_CUB:
        send_cmd(CMD_SEL_CUB);
        break;
    }
}

static void window_load(Window *window) {

    // get window frame
    Layer *window_layer = window_get_root_layer(window);

    // 画像表示レイヤー初期化(上段)
    // コントロールアイコン
    train_window_data.control_icon_layer = bitmap_layer_create(GRect(95, 11, 46, 46));
    bitmap_layer_set_bitmap(train_window_data.control_icon_layer, train_window_data.control_icon_bitmap);

    //GRect bounds = layer_get_frame(window_layer);
    layer_add_child(window_layer, bitmap_layer_get_layer(train_window_data.control_icon_layer));
    // トレーニングアイコン
    train_window_data.training_icon_layer = bitmap_layer_create(GRect(3, 11, 46, 46));
    bitmap_layer_set_bitmap(train_window_data.training_icon_layer, train_window_data.training_icon_bitmap);
    layer_add_child(window_layer, bitmap_layer_get_layer(train_window_data.training_icon_layer));

    // 値表示レイヤー初期化（中段）
    train_window_data.mdlValLayer = text_layer_create(GRect(0, 57, 144, 54));
    text_layer_set_text(train_window_data.mdlValLayer, train_window_data.valDuration);
    text_layer_set_text_alignment(train_window_data.mdlValLayer, GTextAlignmentCenter);
    text_layer_set_font(train_window_data.mdlValLayer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SF_SQUARE_HEAD_CONDENSED_40)));
    layer_add_child(window_layer, text_layer_get_layer(train_window_data.mdlValLayer));
    
    // 値表示レイヤー初期化（下段）
    // 値
    train_window_data.btmValLayer = text_layer_create(GRect(0, 111, 95, 54));
    text_layer_set_text(train_window_data.btmValLayer, train_window_data.valBottom);
    text_layer_set_text_alignment(train_window_data.btmValLayer, GTextAlignmentRight);
    text_layer_set_font(train_window_data.btmValLayer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SF_SQUARE_HEAD_CONDENSED_40)));
    layer_add_child(window_layer, text_layer_get_layer(train_window_data.btmValLayer));
    // 単位
    train_window_data.btmUntLayer = text_layer_create(GRect(96, 130, 46, 54));
    text_layer_set_text(train_window_data.btmUntLayer, train_window_data.untBottom);
    text_layer_set_text_alignment(train_window_data.btmUntLayer, GTextAlignmentLeft);
    text_layer_set_font(train_window_data.btmUntLayer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SF_SQUARE_HEAD_CONDENSED_20)));
    layer_add_child(window_layer, text_layer_get_layer(train_window_data.btmUntLayer));

    // 反転レイヤー
    train_window_data.invLayer = inverter_layer_create(
            GRect(0, 0, 144, 168));
    layer_add_child(window_layer,
            inverter_layer_get_layer(train_window_data.invLayer));

    // フォーカスの初期化(右上)
    train_window_data.indexFocus = TRAIN_WINDOW_FOCUS_UPPER_RIGHT;
    // フォーカスレイヤー左上
    train_window_data.focusUpperLeftLayer = inverter_layer_create(
            GRect(3, 11, 46, 46));
    layer_add_child(window_layer,
            inverter_layer_get_layer(train_window_data.focusUpperLeftLayer));
    // フォーカスレイヤー右上
    train_window_data.focusUpperRightLayer = inverter_layer_create(
            GRect(95, 11, 46, 46));
    layer_add_child(window_layer,
            inverter_layer_get_layer(train_window_data.focusUpperRightLayer));
    // フォーカスレイヤー中段
    train_window_data.focusMiddleLayer = inverter_layer_create(
            GRect(3, 61, 138, 46));
    layer_add_child(window_layer,
            inverter_layer_get_layer(train_window_data.focusMiddleLayer));
    // フォーカスレイヤー下段
    train_window_data.focusBottomLayer = inverter_layer_create(
            GRect(3, 115, 138, 46));
    layer_add_child(window_layer,
            inverter_layer_get_layer(train_window_data.focusBottomLayer));

    // フォーカス更新
    update_focus();
    // 継続時間更新
    update_duration();

    // PhoneAppからのデータ受信コールバック登録
    Tuplet initial_values[] = {
            TupletInteger(KEY_STATUS, STATUS_STOP),
            TupletInteger(KEY_TRAINING_TYPE, TYPE_RUN),
            TupletInteger(KEY_VAL_DURATION, 0),
            TupletCString(KEY_VAL_BOTTOM, "0.00"),
            TupletCString(KEY_UNT_BOTTOM, "km"),
            TupletInteger(KEY_CONTROL_STOPWATCH, CMD_NO_CONTROL),
            TupletInteger(KEY_GET_VERSION, 0),
    };
    app_sync_init(
            &train_window_data.sync, 
            train_window_data.sync_buffer, 
            sizeof(train_window_data.sync_buffer), 
            initial_values, 
            ARRAY_LENGTH(initial_values), 
            sync_tuple_changed_callback, 
            sync_error_callback, 
            NULL);

    // PhoneAppに起動通知→PhoneAppが状態を返してPhoneAppの画面更新したい
    train_window_data.displayItemBottom = CMD_GET_DISTANCE;
    send_cmd(train_window_data.displayItemBottom);
}

static void window_unload(Window *window) {
    // データ送受信の後始末
    app_sync_deinit(&train_window_data.sync);
    // UIインスタンスの後始末
    bitmap_layer_destroy(train_window_data.control_icon_layer);
    bitmap_layer_destroy(train_window_data.training_icon_layer);
    text_layer_destroy(train_window_data.mdlValLayer);
    text_layer_destroy(train_window_data.btmValLayer);
    text_layer_destroy(train_window_data.btmUntLayer);
    inverter_layer_destroy(train_window_data.invLayer);
    inverter_layer_destroy(train_window_data.focusUpperLeftLayer);
    inverter_layer_destroy(train_window_data.focusUpperRightLayer);
    inverter_layer_destroy(train_window_data.focusMiddleLayer);
    inverter_layer_destroy(train_window_data.focusBottomLayer);
}

/**
 * 初期化
 */
void train_window_handle_init(void) {

    // 画面の初期化
    trainWindow = window_create();
    window_set_window_handlers(trainWindow, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    window_set_fullscreen(trainWindow, true);
//    window_set_background_color(&trainWindow, GColorBlack);
    window_set_click_config_provider_with_context(trainWindow,
            train_window_click_config_provider, (void*)trainWindow);

    // とりあえず、単位系はSIにしておく。本来はPhoneAppから取得した値で更新する
    train_window_data.unitSystem = UNIT_SYSTEM_SI;
    init_stopwatch();

    // 画面表示
    window_stack_push(trainWindow, true /* Animated */);
}

/**
 * 後始末
 */
void train_window_handle_deinit(void) {
    window_destroy(trainWindow);
}

/**
 * 継続時間を更新
 */
void update_duration() {
    // 時分秒に変換
    int seconds = (train_window_data.elapsed_time / 1000) % 60;
    int minutes = (train_window_data.elapsed_time / 60000) % 60;
    int hours = train_window_data.elapsed_time / 3600000;
    
    // 表示更新
    static char timeText[] = "00:00:00";
    itoa2(hours, &timeText[0]);
    itoa2(minutes, &timeText[3]);
    itoa2(seconds, &timeText[6]);

    train_window_data.valDuration = timeText;
    update_disp_item_middle(ITEM_DURATION);
}

/**
 * ストップウォッチが動いている間だけ呼び出される
 * 秒を刻むイベントハンドラ
 */
void handle_timer(void *data) {
    if (train_window_data.started) {
        layer_mark_dirty(text_layer_get_layer(train_window_data.btmValLayer));

        train_window_data.elapsed_time += 100;
        time_t pebble_time = get_pebble_time();
        if (!train_window_data.last_pebble_time) train_window_data.last_pebble_time = pebble_time;
        if (pebble_time > train_window_data.last_pebble_time) {
            if (!train_window_data.start_time) {
                train_window_data.start_time = pebble_time - train_window_data.elapsed_time;
            } else {
                train_window_data.elapsed_time = pebble_time - train_window_data.start_time;
            }
            train_window_data.last_pebble_time = pebble_time;
        }
        train_window_data.update_timer = app_timer_register(1000, handle_timer, NULL);
    }
    // 継続時間表示の場合、継続時間で画面表示更新
    if (train_window_data.displayItemMiddle == ITEM_DURATION) {
        update_duration();
    }
}

/**
 * 常に実行され続ける秒間イベント
 */
void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {

    if (train_window_data.displayItemMiddle == ITEM_TIMESTAMP) {
        time_t timer;
        struct tm *t;
        // 現在時刻取得
        time(&timer);
        t = localtime(&timer);
        strftime(
                train_window_data.valTimestamp,
                sizeof(train_window_data.valTimestamp),
                "%T",
                t);
        update_disp_item_middle(train_window_data.displayItemMiddle);
    }

}
