#include "fastfetch.h"

#define FF_SONG_MODULE_NAME "Song"
#define FF_SONG_NUM_FORMAT_ARGS 3

void ffPrintSong(FFinstance* instance)
{
    const FFMediaResult* media = ffDetectMedia(instance);

    if(media->song.length == 0)
    {
        ffPrintError(instance, FF_SONG_MODULE_NAME, 0, &instance->config.songKey, &instance->config.songFormat, FF_SONG_NUM_FORMAT_ARGS, "No song detected");
        return;
    }

    if(instance->config.songFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_SONG_MODULE_NAME, 0, &instance->config.songKey);

        if(media->artist.length > 0)
        {
            ffStrbufWriteTo(&media->artist, stdout);
            fputs(" - ", stdout);
        }

        if(media->album.length > 0)
        {
            ffStrbufWriteTo(&media->album, stdout);
            fputs(" - ", stdout);
        }

        ffStrbufPutTo(&media->song, stdout);
    }
    else
    {
        ffPrintFormatString(instance, FF_SONG_MODULE_NAME, 0, &instance->config.songKey, &instance->config.songFormat, NULL, FF_SONG_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->song},
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->artist},
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->album}
        });
    }
}
