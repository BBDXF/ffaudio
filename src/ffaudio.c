#include "ffaudio.h"
#include <dshow.h>
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
#include <pthread.h>
#include <windows.h>

typedef struct
{
    ffaudio_info_t info;
    ffaudio_cb_info cb_info;

    // mutex
    // pthread_mutex_t mutex;
    pthread_t thread;
    // ffmpeg
    AVFormatContext *format_context;
    AVCodecContext *codec_context;
    int audio_stream_index;
    // dshow
    IGraphBuilder *pGraph;
    // IMediaControl *pControl;
    // IMediaEventEx *pEvent;
    IBaseFilter *pRenderer;
} ffaudio_player_t;

void ffaudio_init()
{
    // Initialize COM for DirectShow
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
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

    // init mutex
    // pthread_mutex_init(&player->mutex, NULL);

    // Create DirectShow filter graph
    CoCreateInstance(&CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, &IID_IGraphBuilder, (void **)&player->pGraph);

    // Create audio renderer
    CoCreateInstance(&CLSID_DSoundRender, NULL, CLSCTX_INPROC_SERVER, &IID_IBaseFilter, (void **)&player->pRenderer);

    // Add renderer to graph
    player->pGraph->lpVtbl->AddFilter(player->pGraph, player->pRenderer, L"Audio Renderer");

    return player;
}
void ffaudio_player_release(void *player)
{
    ffaudio_player_t *p = (ffaudio_player_t *)player;
    // if (p->mutex)
    // {
    //     pthread_mutex_destroy(&p->mutex);
    // }
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

static void *ffaudio_player_thread(void *arg)
{
    ffaudio_player_t *p = (ffaudio_player_t *)arg;
    AVPacket packet;
    AVFrame *frame = av_frame_alloc();
    int ret = 0;

    // dshow
    // Create media sample using allocator
    IMemAllocator *pAlloc = NULL;
    IMemInputPin *pInputPin = NULL;
    IMediaSample *pSample = NULL;
    
    // Get input pin from renderer
    p->pRenderer->lpVtbl->FindPin(p->pRenderer, L"In", &pInputPin);
    pInputPin->lpVtbl->GetAllocator(pInputPin, &pAlloc);
    
    // Create sample
    pAlloc->lpVtbl->GetBuffer(pAlloc, &pSample, NULL, NULL, 0);


    // Main decoding loop
    while ((av_read_frame(p->format_context, &packet)) >= 0) {
        if (packet.stream_index == p->audio_stream_index) {
            avcodec_send_packet(p->codec_context, &packet);
            
            while (avcodec_receive_frame(p->codec_context, frame) >= 0) {
                // Get audio data pointer and size
                BYTE *pData = frame->data[0];
                long cbData = frame->nb_samples * frame->ch_layout.nb_channels * 2;
                
                // Create media sample and send to renderer
                pSample->lpVtbl->SetPointer(pSample, pData, cbData);
    
                // Set properties
                pSample->lpVtbl->SetActualDataLength(pSample, cbData);
                pSample->lpVtbl->SetTime(pSample, NULL, NULL);
                
                // Send to renderer
                pInputPin->lpVtbl->Receive(pInputPin, pSample);
            }
        }
        av_packet_unref(&packet);
    }

    // Release interfaces
    pSample->lpVtbl->Release(pSample);
    pAlloc->lpVtbl->Release(pAlloc);
    pInputPin->lpVtbl->Release(pInputPin);

    // Cleanup
    av_frame_free(&frame);
    return NULL;
}

int ffaudio_player_load(void *player, const char *url, const char *headers)
{
    ffaudio_player_t *p = (ffaudio_player_t *)player;
    AVDictionaryEntry *tag = NULL;
    int ret = 0;

    if (p->info.state != FFAUDIO_STATE_IDLE)
    {
        return -1;
    }

    // pthread_mutex_lock(&p->mutex);
    // 打开输入文件
    ret = avformat_open_input(&p->format_context, url, NULL, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not open input file: %s\n", av_err2str(ret));
        // pthread_mutex_unlock(&p->mutex);
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
        // pthread_mutex_unlock(&p->mutex);
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
        // pthread_mutex_unlock(&p->mutex);
        return AVERROR_STREAM_NOT_FOUND;
    }

    p->info.audio_duration = p->format_context->duration / (AV_TIME_BASE / 1000);

    // 获取音频流信息
    AVStream *audio_stream = p->format_context->streams[p->audio_stream_index];
    p->info.audio_sample_rate = audio_stream->codecpar->sample_rate;
    p->info.audio_channels = audio_stream->codecpar->ch_layout.nb_channels;
    p->info.audio_bit_rate = audio_stream->codecpar->bit_rate;
    p->info.audio_sample_size = audio_stream->codecpar->format;

    p->info.state = FFAUDIO_STATE_LOADED;
    // unlock
    // pthread_mutex_unlock(&p->mutex);

    // play in thread
    if (pthread_create(&p->thread, NULL, ffaudio_player_thread, p))
    {
        fprintf(stderr, "Could not create audio play thread.\n");
        return -1;
    }

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