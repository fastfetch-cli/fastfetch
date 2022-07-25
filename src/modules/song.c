#include "fastfetch.h"
#include "common/printing.h"

#include <ctype.h>

#define FF_SONG_MODULE_NAME "Song"
#define FF_SONG_NUM_FORMAT_ARGS 5

static bool shouldIgoreChar(char c)
{
    return isblank(c) || c == '-' || c == '.';
}

static bool artistInSongTitle(const FFstrbuf* song, const FFstrbuf* artist)
{
    uint32_t artistIndex = 0;
    uint32_t songIndex = 0;

    while(true)
    {
        while(shouldIgoreChar(song->chars[songIndex]))
            ++songIndex;

        while(shouldIgoreChar(artist->chars[artistIndex]))
            ++artistIndex;

        if(artist->chars[artistIndex] == '\0')
            return true;

        if(song->chars[songIndex] == '\0')
            return false;

        if(tolower(song->chars[songIndex]) != tolower(artist->chars[artistIndex]))
            return false;

        ++artistIndex;
        ++songIndex;
    }

    //Unreachable
    return false;
}

void ffPrintSong(FFinstance* instance)
{
    const FFMediaResult* media = ffDetectMedia(instance);

    if(media->song.length == 0)
    {
        ffPrintError(instance, FF_SONG_MODULE_NAME, 0, &instance->config.song, "No song detected");
        return;
    }

    FFstrbuf songPretty;
    ffStrbufInitCopy(&songPretty, &media->song);
    const char* removeStrings[] = {
        "(Official Music Video)", "(Official Video)", "(Music Video)",
        "[Official Music Video]", "[Official Video]", "[Music Video]",
        "| Official Music Video", "| Official Video", "| Music Video",
        "[Official Audio]", "[Audio]", "(Audio)", "| Official Audio", "| Audio", "| OFFICIAL AUDIO",
        "(Lyric Video)", "(Official Lyric Video)", "(Lyrics)",
        "[Lyric Video]", "[Official Lyric Video]", "[Lyrics]",
        "| Lyric Video", "| Official Lyric Video", "| Lyrics",
    };
    ffStrbufRemoveStringsA(&songPretty, sizeof(removeStrings) / sizeof(removeStrings[0]), removeStrings);
    ffStrbufTrimRight(&songPretty, ' ');

    if(songPretty.length == 0)
        ffStrbufAppend(&songPretty, &media->song);

    if(instance->config.song.outputFormat.length == 0)
    {
        //We don't expose artistPretty to the format, as it might be empty (when the think that the artist is already in the song title)
        FFstrbuf artistPretty;
        ffStrbufInitCopy(&artistPretty, &media->artist);
        ffStrbufRemoveIgnCaseEndS(&artistPretty, " - Topic");
        ffStrbufRemoveIgnCaseEndS(&artistPretty, "VEVO");
        ffStrbufTrimRight(&artistPretty, ' ');

        if(artistInSongTitle(&songPretty, &artistPretty))
            ffStrbufClear(&artistPretty);

        ffPrintLogoAndKey(instance, FF_SONG_MODULE_NAME, 0, &instance->config.song.key);

        if(artistPretty.length > 0)
        {
            ffStrbufWriteTo(&artistPretty, stdout);
            fputs(" - ", stdout);
        }

        ffStrbufPutTo(&songPretty, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_SONG_MODULE_NAME, 0, &instance->config.song, FF_SONG_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &songPretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->song},
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->artist},
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->album},
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->url}
        });
    }
}
