#include <pebble.h>
#include <time.h>

/**
 * intを2桁の文字列に変換して返す。
 * 99を超えたら99に変換する。
 */
void itoa2(int num, char* buffer) {
    const char digits[10] = "0123456789";
    if (num > 99) {
        buffer[0] = '9';
        buffer[1] = '9';
        return;
    } else if (num > 9) {
        buffer[0] = digits[num / 10];
    } else {
        buffer[0] = '0';
    }
    buffer[1] = digits[num % 10];
}

/**
 * 現在時刻をtime_t型で返す
 */
time_t get_pebble_time() {
    time_t timer;
    struct tm *t;

    // 現在時刻取得
    time(&timer);
    t = localtime(&timer);

    time_t seconds = t->tm_sec;
    seconds += t->tm_min * 60;
    seconds += t->tm_hour * 3600;
    seconds += t->tm_yday * 86400;
    seconds += (t->tm_year - 2012) * 31536000;
    return seconds * 1000;
}
