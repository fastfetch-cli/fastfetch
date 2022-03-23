#include "fastfetch.h"

#include <ctype.h>

#define FF_PLAYER_MODULE_NAME "Media Player"
#define FF_PLAYER_NUM_FORMAT_ARGS 3

void ffPrintPlayer(FFinstance* instance)
{
    const FFMediaResult* media = ffDetectMedia(instance);

    if(media->player.length == 0)
    {
        ffPrintError(instance, FF_PLAYER_MODULE_NAME, 0, &instance->config.playerKey, &instance->config.playerFormat, FF_PLAYER_NUM_FORMAT_ARGS, "No media player found");
        return;
    }

    FFstrbuf playerPretty;
    ffStrbufInit(&playerPretty);

    //If we are on a website, prepend the website name
    if(ffStrbufStartsWithS(&media->url, "https://www."))
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

    if(instance->config.playerFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_PLAYER_MODULE_NAME, 0, &instance->config.playerKey);
        ffStrbufPutTo(&playerPretty, stdout);
    }
    else
    {
        ffPrintFormatString(instance, FF_PLAYER_MODULE_NAME, 0, &instance->config.playerKey, &instance->config.playerFormat, NULL, FF_PLAYER_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &playerPretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->player},
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->busNameShort}
        });
    }
}
