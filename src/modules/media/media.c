#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/media/media.h"
#include "modules/media/media.h"
#include "util/stringUtils.h"

#include <ctype.h>

static inline bool shouldIgnoreChar(char c)
{
    return isblank(c) || c == '-' || c == '.';
}

static bool artistInSongTitle(const FFstrbuf* song, const FFstrbuf* artist)
{
    uint32_t artistIndex = 0;
    uint32_t songIndex = 0;

    while(true)
    {
        while(shouldIgnoreChar(song->chars[songIndex]))
            ++songIndex;

        while(shouldIgnoreChar(artist->chars[artistIndex]))
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

bool ffPrintMedia(FFMediaOptions* options)
{
    const FFMediaResult* media = ffDetectMedia();

    if(media->error.length > 0)
    {
        ffPrintError(FF_MEDIA_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", media->error.chars);
        return false;
    }

    FF_STRBUF_AUTO_DESTROY songPretty = ffStrbufCreateCopy(&media->song);
    const char* removeStrings[] = {
        "(Official Music Video)", "(Official Video)", "(Music Video)", "(Official HD Video)",
        "[Official Music Video]", "[Official Video]", "[Music Video]", "[Official HD Video]",
        "| Official Music Video", "| Official Video", "| Music Video", "| Official HD Video",
        "[Official Audio]", "[Audio]", "(Audio)", "| Official Audio", "| Audio", "| OFFICIAL AUDIO",
        "(Lyric Video)", "(Official Lyric Video)", "(Lyrics)",
        "[Lyric Video]", "[Official Lyric Video]", "[Lyrics]",
        "| Lyric Video", "| Official Lyric Video", "| Lyrics",
    };
    ffStrbufRemoveStrings(&songPretty, ARRAY_SIZE(removeStrings), removeStrings);
    ffStrbufTrimRight(&songPretty, ' ');

    if(songPretty.length == 0)
        ffStrbufAppend(&songPretty, &media->song);

    if(options->moduleArgs.outputFormat.length == 0)
    {
        //We don't expose artistPretty to the format, as it might be empty (when the think that the artist is already in the song title)
        FF_STRBUF_AUTO_DESTROY artistPretty = ffStrbufCreateCopy(&media->artist);
        ffStrbufRemoveIgnCaseEndS(&artistPretty, " - Topic");
        ffStrbufRemoveIgnCaseEndS(&artistPretty, "VEVO");
        ffStrbufTrimRight(&artistPretty, ' ');

        if(artistInSongTitle(&songPretty, &artistPretty))
            ffStrbufClear(&artistPretty);

        ffPrintLogoAndKey(FF_MEDIA_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        if(artistPretty.length > 0)
        {
            ffStrbufWriteTo(&artistPretty, stdout);
            fputs(" - ", stdout);
        }

        if (media->status.length > 0)
            ffStrbufAppendF(&songPretty, " (%s)", media->status.chars);

        ffStrbufPutTo(&songPretty, stdout);
    }
    else
    {
        FF_PRINT_FORMAT_CHECKED(FF_MEDIA_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
            FF_FORMAT_ARG(songPretty, "combined"),
            FF_FORMAT_ARG(media->song, "title"),
            FF_FORMAT_ARG(media->artist, "artist"),
            FF_FORMAT_ARG(media->album, "album"),
            FF_FORMAT_ARG(media->status, "status"),
            FF_FORMAT_ARG(media->player, "player-name"),
            FF_FORMAT_ARG(media->playerId, "player-id"),
            FF_FORMAT_ARG(media->url, "url"),
        }));
    }

    return true;
}

void ffParseMediaJsonObject(FFMediaOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_MEDIA_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateMediaJsonConfig(FFMediaOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateMediaJsonResult(FF_MAYBE_UNUSED FFMediaOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    const FFMediaResult* media = ffDetectMedia();

    if(media->error.length > 0)
    {
        yyjson_mut_obj_add_strbuf(doc, module, "error", &media->error);
        return false;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");

    yyjson_mut_val* song = yyjson_mut_obj_add_obj(doc, obj, "song");
    yyjson_mut_obj_add_strbuf(doc, song, "name", &media->song);
    yyjson_mut_obj_add_strbuf(doc, song, "artist", &media->artist);
    yyjson_mut_obj_add_strbuf(doc, song, "album", &media->album);
    yyjson_mut_obj_add_strbuf(doc, song, "status", &media->status);

    yyjson_mut_val* player = yyjson_mut_obj_add_obj(doc, obj, "player");
    yyjson_mut_obj_add_strbuf(doc, player, "name", &media->player);
    yyjson_mut_obj_add_strbuf(doc, player, "id", &media->playerId);
    yyjson_mut_obj_add_strbuf(doc, player, "url", &media->url);

    return true;
}

void ffInitMediaOptions(FFMediaOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");
}

void ffDestroyMediaOptions(FFMediaOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffMediaModuleInfo = {
    .name = FF_MEDIA_MODULE_NAME,
    .description = "Print playing song name",
    .initOptions = (void*) ffInitMediaOptions,
    .destroyOptions = (void*) ffDestroyMediaOptions,
    .parseJsonObject = (void*) ffParseMediaJsonObject,
    .printModule = (void*) ffPrintMedia,
    .generateJsonResult = (void*) ffGenerateMediaJsonResult,
    .generateJsonConfig = (void*) ffGenerateMediaJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Pretty media name", "combined"},
        {"Media name", "title"},
        {"Artist name", "artist"},
        {"Album name", "album"},
        {"Status", "status"},
    }))
};
