#ifndef __FF_AUDIO_H__
#define __FF_AUDIO_H__
#include <math.h>
#include <stdint.h>

#define FFAUDIO_STATE_IDLE 0
#define FFAUDIO_STATE_LOAD 1
#define FFAUDIO_STATE_PLAYING 2
#define FFAUDIO_STATE_PAUSED 3

typedef struct
{
    // audio info
    int32_t state; // FFAUDIO_STATE_xxx;
    float_t volume;
    float_t speed;

    int64_t audio_duration; // ms
    int64_t audio_position; // ms

    int32_t audio_bit_rate;    // kbps;
    int32_t audio_sample_rate; // Hz;
    int32_t audio_channels;    // 1, 2;
    int32_t audio_sample_size; // 8, 16;

    // file info
    char url[512];
    char headers[1024];
    // meta data
    char mime_type[32];
    char title[128];
    char artist[128];
    char album[128];
    uint8_t *cover;
    int32_t cover_size;
} ffaudio_info_t;

typedef void (*ffaudio_cb_info)(ffaudio_info_t *info);

// common
void ffaudio_init();
void ffaudio_deinit();

// player
void *ffaudio_player_create(ffaudio_cb_info cb_info);
void ffaudio_player_release(void *player);
int ffaudio_player_load(void *player, const char *url, const char *headers); // 创建一个线程，加载音频
void ffaudio_player_play(void *player, int state);
void ffaudio_player_set_volume(void *player, float volume);
void ffaudio_player_set_speed(void *player, float speed);
void ffaudio_player_seek(void *player, float percent);

// utils
int ffaudio_player_get_info(void *player, ffaudio_info_t *info);

#endif