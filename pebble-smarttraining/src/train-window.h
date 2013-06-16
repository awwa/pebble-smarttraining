#ifndef TRAIN_WINDOW_H
#define TRAIN_WINDOW_H
void train_window_handle_init(AppContextRef ctx);
void train_window_handle_deinit(AppContextRef ctx);
void update_training_type_icon(int trainingType);
void update_disp_item_middle(uint32_t displayItem);
void update_disp_item_bottom(uint32_t displayItem);
void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie);
void train_window_select_control_pause();
void train_window_select_control_stop();
void train_window_select_training_type(int index);
void update_duration();
void start_stopwatch();
void stop_stopwatch();
void restart_stopwatch();
void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t);
#endif // TRAIN_WINDOW_H
