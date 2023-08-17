#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/media/media.h"
#include "modules/player/player.h"
#include "util/stringUtils.h"

#include <ctype.h>

#define FF_PLAYER_DISPLAY_NAME "Media Player"
#define FF_PLAYER_NUM_FORMAT_ARGS 4

void ffPrintPlayer(FFPlayerOptions* options)
{
    const FFMediaResult* media = ffDetectMedia();

    if(media->error.length > 0)
    {
        ffPrintError(FF_PLAYER_DISPLAY_NAME, 0, &options->moduleArgs, "%s", media->error.chars);
        return;
    }

    FF_STRBUF_AUTO_DESTROY playerPretty = ffStrbufCreate();

    //If we are on a website, prepend the website name
    if(
        ffStrbufIgnCaseCompS(&media->playerId, "spotify") == 0 ||
        ffStrbufIgnCaseCompS(&media->playerId, "vlc") == 0
    ) {} // do noting, surely not a website, even if the url is set
    else if(ffStrbufStartsWithS(&media->url, "https://www."))
        ffStrbufAppendS(&playerPretty, media->url.chars + 12);
    else if(ffStrbufStartsWithS(&media->url, "http://www."))
        ffStrbufAppendS(&playerPretty, media->url.chars + 11);
    else if(ffStrbufStartsWithS(&media->url, "https://"))
        ffStrbufAppendS(&playerPretty, media->url.chars + 8);
    else if(ffStrbufStartsWithS(&media->url, "http://"))
        ffStrbufAppendS(&playerPretty, media->url.chars + 7);

    //If we found a website name, make it more pretty
    if(playerPretty.length > 0)
    {
        ffStrbufSubstrBeforeFirstC(&playerPretty, '/'); //Remove the path
        ffStrbufSubstrBeforeLastC(&playerPretty, '.'); //Remove the TLD
    }

    //Check again for length, as we may have removed everything.
    bool playerPrettyIsCustom = playerPretty.length > 0;

    //If we don't have subdomains, it is usually more pretty to capitalize the first letter.
    if(playerPrettyIsCustom && ffStrbufFirstIndexC(&playerPretty, '.') == playerPretty.length)
        playerPretty.chars[0] = (char) toupper(playerPretty.chars[0]);

    if(playerPrettyIsCustom)
        ffStrbufAppendS(&playerPretty, " (");

    ffStrbufAppend(&playerPretty, &media->player);

    if(playerPrettyIsCustom)
        ffStrbufAppendC(&playerPretty, ')');

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_PLAYER_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        ffStrbufPutTo(&playerPretty, stdout);
    }
    else
    {
        ffPrintFormat(FF_PLAYER_DISPLAY_NAME, 0, &options->moduleArgs, FF_PLAYER_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &playerPretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->player},
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->playerId},
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->url}
        });
    }
}

void ffInitPlayerOptions(FFPlayerOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_PLAYER_MODULE_NAME, ffParsePlayerCommandOptions, ffParsePlayerJsonObject, ffPrintPlayer);
    ffOptionInitModuleArg(&options->moduleArgs);
}

bool ffParsePlayerCommandOptions(FFPlayerOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_PLAYER_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
}

void ffDestroyPlayerOptions(FFPlayerOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParsePlayerJsonObject(FFPlayerOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        ffPrintError(FF_PLAYER_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}
