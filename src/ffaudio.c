#include "ffaudio.h"
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
// #include <threads.h>

typedef struct
{
    ffaudio_info_t info;
    ffaudio_cb_info cb_info;

    // ffmpeg
    AVFormatContext *format_context;
    AVCodecContext *codec_context;
    int audio_stream_index;
} ffaudio_player_t;

void ffaudio_init()
{
    avformat_network_init();
    avdevice_register_all();
}
void ffaudio_deinit()
{
    avformat_network_deinit();
}

void *ffaudio_player_create(ffaudio_cb_info cb_info)
{
    ffaudio_player_t *player = (ffaudio_player_t *)malloc(sizeof(ffaudio_player_t));
    memset(player, 0, sizeof(ffaudio_player_t));
    player->cb_info = cb_info;
    player->info.state = FFAUDIO_STATE_IDLE;
    player->info.volume = 1.0;
    player->info.speed = 1.0;

    return player;
}
void ffaudio_player_release(void *player)
{
    ffaudio_player_t *p = (ffaudio_player_t *)player;
    if (p->format_context)
    {
        avformat_close_input(&p->format_context);
        p->format_context = NULL;
    }
    if (p->codec_context)
    {
        avcodec_free_context(&p->codec_context);
        p->codec_context = NULL;
    }
    if (p->info.cover)
    {
        free(p->info.cover);
        p->info.cover = NULL;
        p->info.cover_size = 0;
    }
    free(p);
}

int ffaudio_player_load(void *player, const char *url, const char *headers)
{
    ffaudio_player_t *p = (ffaudio_player_t *)player;
    AVDictionaryEntry *tag = NULL;
    int ret = 0;
    // 打开输入文件
    ret = avformat_open_input(&p->format_context, url, NULL, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not open input file: %s\n", av_err2str(ret));
        return ret;
    }
    strncpy(p->info.url, url, 512);
    if (headers != NULL)
    {
        strncpy(p->info.headers, headers, 1024);
    }
    // 获取meta信息
    ret = avformat_find_stream_info(p->format_context, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not find stream information: %s\n", av_err2str(ret));
        return ret;
    }
    if (p->format_context->iformat->name != NULL)
    {
        memcpy(p->info.mime_type, p->format_context->iformat->name, 128);
    }
    // meta信息
    while ((tag = av_dict_get(p->format_context->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
    {
        // printf("%s=%s\n", tag->key, tag->value);
        if (strcmp(tag->key, "title") == 0)
        {
            strncpy(p->info.title, tag->value, 128);
        }
        else if (strcmp(tag->key, "artist") == 0)
        {
            strncpy(p->info.artist, tag->value, 128);
        }
        else if (strcmp(tag->key, "album") == 0)
        {
            strncpy(p->info.album, tag->value, 128);
        }
    }
    // 查找封面
    for (unsigned int i = 0; i < p->format_context->nb_streams; i++)
    {
        if (p->format_context->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC)
        {
            // pic
            AVPacket *cover = &p->format_context->streams[i]->attached_pic;
            if (cover->size > 0)
            {
                p->info.cover_size = cover->size;
                p->info.cover = (uint8_t *)malloc(cover->size);
                memcpy(p->info.cover, cover->data, cover->size);
            }
            break;
        }
    }
    // 查找音频流
    p->audio_stream_index = av_find_best_stream(p->format_context, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

    if (p->audio_stream_index < 0)
    {
        fprintf(stderr, "Could not find audio stream in the input file.\n");
        return AVERROR_STREAM_NOT_FOUND;
    }

    p->info.audio_duration = p->format_context->duration / (AV_TIME_BASE / 1000);

    // 获取音频流信息
    AVStream *audio_stream = p->format_context->streams[p->audio_stream_index];
    p->info.audio_sample_rate = audio_stream->codecpar->sample_rate;
    p->info.audio_channels = audio_stream->codecpar->ch_layout.nb_channels;
    p->info.audio_bit_rate = audio_stream->codecpar->bit_rate;
    p->info.audio_sample_size = audio_stream->codecpar->format;

    return 0;
}

void ffaudio_player_play(void *player, int state)
{
}
void ffaudio_player_set_volume(void *player, float volume)
{
}
void ffaudio_player_set_speed(void *player, float speed)
{
}
void ffaudio_player_seek(void *player, float percent)
{
}

// utils
int ffaudio_player_get_info(void *player, ffaudio_info_t *info)
{
    if (!player || !info)
    {
        return -1;
    }
    memset(info, 0, sizeof(ffaudio_info_t));
    ffaudio_player_t *p = (ffaudio_player_t *)player;
    memcpy(info, &p->info, sizeof(ffaudio_info_t));
    return 0;
}