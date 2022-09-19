#include "fastfetch.h"
#include "common/printing.h"
#include "detection/media/media.h"

#include <ctype.h>

#define FF_PLAYER_MODULE_NAME "Media Player"
#define FF_PLAYER_NUM_FORMAT_ARGS 4

void ffPrintPlayer(FFinstance* instance)
{
    const FFMediaResult* media = ffDetectMedia(instance);

    if(media->player.length == 0)
    {
        ffPrintError(instance, FF_PLAYER_MODULE_NAME, 0, &instance->config.player, "No media player found");
        return;
    }

    FFstrbuf playerPretty;
    ffStrbufInit(&playerPretty);

    //If we are on a website, prepend the website name
    if(
        ffStrbufIgnCaseCompS(&media->busNameShort, "spotify") == 0 ||
        ffStrbufIgnCaseCompS(&media->busNameShort, "vlc") == 0
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

    if(instance->config.player.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_PLAYER_MODULE_NAME, 0, &instance->config.player.key);
        ffStrbufPutTo(&playerPretty, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_PLAYER_MODULE_NAME, 0, &instance->config.player, FF_PLAYER_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &playerPretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->player},
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->busNameShort},
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->url}
        });
    }

    ffStrbufDestroy(&playerPretty);
}
