#include "ffaudio.h"
#include <direct.h>
#include <stdio.h>
#include <windows.h>
int main(int argc, char **argv)
{
    // char buff[512];
    // _getcwd(buff, 512);
    // printf("ffaudio demo, argc: %d, %s\n", argc, buff);
    // for(int i=0; i<argc; i++){
    //     printf("argv[%d]: %s\n", i, argv[i]);
    // }
    if (argc < 2)
    {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }
    // SetConsoleOutputCP(65001);
    // printf("测试中文输出 - Default\n");
    // printf(u8"测试中文输出 - U8\n");
    ffaudio_init();
    void *player = ffaudio_player_create(NULL);
    ffaudio_player_load(player, argv[1], NULL);

    ffaudio_info_t info;
    ffaudio_player_get_info(player, &info);
    {
        printf("url: %s\n", info.url);
        printf("title: %s\n", info.title);
        printf("artist: %s\n", info.artist);
        printf("album: %s\n", info.album);
        printf("duration: %lld ms\n", info.audio_duration);
        printf("sample_rate: %d Hz\n", info.audio_sample_rate);
        printf("channels: %d\n", info.audio_channels);
        printf("bit_rate: %d bps\n", info.audio_bit_rate);
        printf("sample_size: %d bits\n", info.audio_sample_size);
        printf("mime_type: %s\n", info.mime_type);
        printf("cover_size: %d B\n", info.cover_size);
        if (info.cover_size > 0)
        {
            FILE *fp = fopen("cover.jpg", "wb");
            if (fp)
            {
                fwrite(info.cover, 1, info.cover_size, fp);
                fclose(fp);
            }
        }
    }

    ffaudio_player_play(player, FFAUDIO_STATE_PLAYING);
    ffaudio_player_release(player);

    ffaudio_deinit();
    return 0;
}