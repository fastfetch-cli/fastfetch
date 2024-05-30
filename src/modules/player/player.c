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
        ffPrintError(FF_PLAYER_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", media->error.chars);
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
        FF_PRINT_FORMAT_CHECKED(FF_PLAYER_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, FF_PLAYER_NUM_FORMAT_ARGS, ((FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &playerPretty, "player"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->player, "name"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->playerId, "id"},
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->url, "url"}
        }));
    }
}

bool ffParsePlayerCommandOptions(FFPlayerOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_PLAYER_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    return false;
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

        ffPrintError(FF_PLAYER_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGeneratePlayerJsonConfig(FFPlayerOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyPlayerOptions))) FFPlayerOptions defaultOptions;
    ffInitPlayerOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);
}

void ffGeneratePlayerJsonResult(FF_MAYBE_UNUSED FFMediaOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    const FFMediaResult* media = ffDetectMedia();

    if(media->error.length > 0)
    {
        yyjson_mut_obj_add_strbuf(doc, module, "error", &media->error);
        return;
    }

    yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
    yyjson_mut_obj_add_strbuf(doc, obj, "player", &media->player);
    yyjson_mut_obj_add_strbuf(doc, obj, "playerId", &media->playerId);
    yyjson_mut_obj_add_strbuf(doc, obj, "url", &media->url);
}

void ffPrintPlayerHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_PLAYER_MODULE_NAME, "{1}", FF_PLAYER_NUM_FORMAT_ARGS, ((const char* []) {
        "Pretty player name - player",
        "Player name - name",
        "Player Identifier - id",
        "URL name - url",
    }));
}

void ffInitPlayerOptions(FFPlayerOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_PLAYER_MODULE_NAME,
        "Print music player name",
        ffParsePlayerCommandOptions,
        ffParsePlayerJsonObject,
        ffPrintPlayer,
        ffGeneratePlayerJsonResult,
        ffPrintPlayerHelpFormat,
        ffGeneratePlayerJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs);
}

void ffDestroyPlayerOptions(FFPlayerOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
