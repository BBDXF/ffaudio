#ifndef __FF_AUDIO_H__
#define __FF_AUDIO_H__
#include <math.h>
#include <stdint.h>

#define FFAUDIO_STATE_IDLE    0
#define FFAUDIO_STATE_LOADED  1
#define FFAUDIO_STATE_PLAYING 2
#define FFAUDIO_STATE_PAUSED  3

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

typedef struct ffaudio_player ffaudio_player;
typedef struct AudioPlayer AudioPlayer;

/* OpenAL播放控制接口 */
int load_audio(const char *filename, AudioPlayer **player);
void play_audio(AudioPlayer *player);
void set_playback_speed(AudioPlayer *player, float speed);
void set_volume(AudioPlayer *player, float volume);

// utils
int ffaudio_player_get_info(void *player, ffaudio_info_t *info);

#endif