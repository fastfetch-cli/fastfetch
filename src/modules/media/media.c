#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/percent.h"
#include "detection/media/media.h"
#include "modules/media/media.h"

#include <ctype.h>

static inline bool shouldIgnoreChar(char c) {
    return isblank(c) || c == '-' || c == '.';
}

static bool artistInSongTitle(const FFstrbuf* song, const FFstrbuf* artist) {
    uint32_t artistIndex = 0;
    uint32_t songIndex = 0;

    while (true) {
        while (shouldIgnoreChar(song->chars[songIndex])) {
            ++songIndex;
        }

        while (shouldIgnoreChar(artist->chars[artistIndex])) {
            ++artistIndex;
        }

        if (artist->chars[artistIndex] == '\0') {
            return true;
        }

        if (song->chars[songIndex] == '\0') {
            return false;
        }

        if (tolower(song->chars[songIndex]) != tolower(artist->chars[artistIndex])) {
            return false;
        }

        ++artistIndex;
        ++songIndex;
    }

    // Unreachable
    return false;
}

static void appendProgress(FFstrbuf* buffer, const FFMediaResult* media) {
    uint32_t sLen = media->length / 1000, sPos = media->position / 1000;
    if (sLen > 60 * 60) {
        ffStrbufAppendF(buffer, "%02u:%02u:%02u / %02u:%02u:%02u", sPos / 3600, (sPos % 3600) / 60, sPos % 60, sLen / 3600, (sLen % 3600) / 60, sLen % 60);
    } else {
        ffStrbufAppendF(buffer, "%02u:%02u / %02u:%02u", sPos / 60, sPos % 60, sLen / 60, sLen % 60);
    }
}

bool ffPrintMedia(FFMediaOptions* options) {
    const FFMediaResult* media = ffDetectMedia(false);

    if (media->error.length > 0) {
        ffPrintError(FF_MEDIA_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", media->error.chars);
        return false;
    }

    FF_STRBUF_AUTO_DESTROY songPretty = ffStrbufCreateCopy(&media->song);
    const char* removeStrings[] = {
        "(Official Music Video)",
        "(Official Video)",
        "(Music Video)",
        "(Official HD Video)",
        "[Official Music Video]",
        "[Official Video]",
        "[Music Video]",
        "[Official HD Video]",
        "| Official Music Video",
        "| Official Video",
        "| Music Video",
        "| Official HD Video",
        "[Official Audio]",
        "[Audio]",
        "(Audio)",
        "| Official Audio",
        "| Audio",
        "| OFFICIAL AUDIO",
        "(Lyric Video)",
        "(Official Lyric Video)",
        "(Lyrics)",
        "[Lyric Video]",
        "[Official Lyric Video]",
        "[Lyrics]",
        "| Lyric Video",
        "| Official Lyric Video",
        "| Lyrics",
    };
    ffStrbufRemoveStrings(&songPretty, ARRAY_SIZE(removeStrings), removeStrings);
    ffStrbufTrimRight(&songPretty, ' ');

    if (songPretty.length == 0) {
        ffStrbufAppend(&songPretty, &media->song);
    }

    FFPercentageTypeFlags percentType = options->percent.type == 0 ? instance.config.display.percentType : options->percent.type;

    if (options->moduleArgs.outputFormat.length == 0) {
        // We don't expose artistPretty to the format, as it might be empty (when the think that the artist is already in the song title)
        FF_STRBUF_AUTO_DESTROY artistPretty = ffStrbufCreateCopy(&media->artist);
        ffStrbufRemoveIgnCaseEndS(&artistPretty, " - Topic");
        ffStrbufRemoveIgnCaseEndS(&artistPretty, "VEVO");
        ffStrbufTrimRight(&artistPretty, ' ');

        if (artistInSongTitle(&songPretty, &artistPretty)) {
            ffStrbufClear(&artistPretty);
        }

        ffPrintLogoAndKey(FF_MEDIA_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        if (artistPretty.length > 0) {
            ffStrbufWriteTo(&artistPretty, stdout);
            fputs(" - ", stdout);
        }

        if (media->length > 0) {
            if (!(percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT)) {
                ffStrbufAppendS(&songPretty, " - ");
                appendProgress(&songPretty, media);
            }

            if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT) {
                ffStrbufAppendC(&songPretty, ' ');
                ffPercentAppendNum(
                    &songPretty,
                    media->position * 100.0 / media->length,
                    options->percent,
                    true,
                    &options->moduleArgs);
            }
            if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT) {
                ffStrbufAppendC(&songPretty, ' ');
                ffPercentAppendBar(
                    &songPretty,
                    media->position * 100.0 / media->length,
                    options->percent,
                    &options->moduleArgs);
            }
        }

        if (media->status.length > 0) {
            ffStrbufAppendF(&songPretty, " [%s]", media->status.chars);
        }

        ffStrbufPutTo(&songPretty, stdout);
    } else {
        FF_STRBUF_AUTO_DESTROY progress = ffStrbufCreate();
        FF_STRBUF_AUTO_DESTROY percentageNum = ffStrbufCreate();
        FF_STRBUF_AUTO_DESTROY percentageBar = ffStrbufCreate();
        if (media->length > 0) {
            if (!(percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT)) {
                appendProgress(&progress, media);
            }
            if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT) {
                ffPercentAppendNum(
                    &percentageNum,
                    media->position * 100.0 / media->length,
                    options->percent,
                    false,
                    &options->moduleArgs);
            }
            if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT) {
                ffPercentAppendBar(
                    &percentageBar,
                    media->position * 100.0 / media->length,
                    options->percent,
                    &options->moduleArgs);
            }
        }
        FF_PRINT_FORMAT_CHECKED(FF_MEDIA_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]){
                                                                                                          FF_ARG(songPretty, "combined"),
                                                                                                          FF_ARG(media->song, "title"),
                                                                                                          FF_ARG(media->artist, "artist"),
                                                                                                          FF_ARG(media->album, "album"),
                                                                                                          FF_ARG(media->status, "status"),
                                                                                                          FF_ARG(progress, "progress"),
                                                                                                          FF_ARG(percentageNum, "progress-num"),
                                                                                                          FF_ARG(percentageBar, "progress-bar"),
                                                                                                          FF_ARG(media->player, "player-name"),
                                                                                                          FF_ARG(media->playerId, "player-id"),
                                                                                                          FF_ARG(media->url, "url"),
                                                                                                      }));
    }

    return true;
}

void ffParseMediaJsonObject(FFMediaOptions* options, yyjson_val* module) {
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach (module, idx, max, key, val) {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs)) {
            continue;
        }

        if (ffPercentParseJsonObject(key, val, &options->percent)) {
            continue;
        }

        ffPrintError(FF_MEDIA_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateMediaJsonConfig(FFMediaOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    ffPercentGenerateJsonConfig(doc, module, options->percent);
}

bool ffGenerateMediaJsonResult(FF_A_UNUSED FFMediaOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    const FFMediaResult* media = ffDetectMedia(false);

    if (media->error.length > 0) {
        yyjson_mut_obj_add_strbuf(doc, module, "error", &media->error);
        return false;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");

    yyjson_mut_val* song = yyjson_mut_obj_add_obj(doc, obj, "song");
    yyjson_mut_obj_add_strbuf(doc, song, "name", &media->song);
    yyjson_mut_obj_add_strbuf(doc, song, "artist", &media->artist);
    yyjson_mut_obj_add_strbuf(doc, song, "album", &media->album);
    yyjson_mut_obj_add_strbuf(doc, song, "status", &media->status);
    yyjson_mut_obj_add_uint(doc, song, "length", media->length);
    yyjson_mut_obj_add_uint(doc, song, "position", media->position);
    if (media->cover.length > 0) {
        yyjson_mut_obj_add_strbuf(doc, song, "cover", &media->cover);
    } else {
        yyjson_mut_obj_add_null(doc, song, "cover");
    }

    yyjson_mut_val* player = yyjson_mut_obj_add_obj(doc, obj, "player");
    yyjson_mut_obj_add_strbuf(doc, player, "name", &media->player);
    yyjson_mut_obj_add_strbuf(doc, player, "id", &media->playerId);
    yyjson_mut_obj_add_strbuf(doc, player, "url", &media->url);

    return true;
}

void ffInitMediaOptions(FFMediaOptions* options) {
    ffOptionInitModuleArg(&options->moduleArgs, "");

    options->percent = (FFPercentageModuleConfig){ 100, 100, 0 };
}

void ffDestroyMediaOptions(FFMediaOptions* options) {
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffMediaModuleInfo = {
    .name = FF_MEDIA_MODULE_NAME,
    .description = "Print the name of the currently playing song",
    .initOptions = (void*) ffInitMediaOptions,
    .destroyOptions = (void*) ffDestroyMediaOptions,
    .parseJsonObject = (void*) ffParseMediaJsonObject,
    .printModule = (void*) ffPrintMedia,
    .generateJsonResult = (void*) ffGenerateMediaJsonResult,
    .generateJsonConfig = (void*) ffGenerateMediaJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]){
        { "Pretty media name", "combined" },
        { "Media name", "title" },
        { "Artist name", "artist" },
        { "Album name", "album" },
        { "Status", "status" },
        { "Progress in text", "progress" },
        { "Progress in percentage (number)", "progress-num" },
        { "Progress in percentage (bar)", "progress-bar" },
        { "Player name", "player-name" },
        { "Player ID", "player-id" },
        { "URL", "url" },
    }))
};
