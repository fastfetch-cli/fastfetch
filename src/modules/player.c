#include "fastfetch.h"

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

    if(instance->config.playerFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_PLAYER_MODULE_NAME, 0, &instance->config.playerKey);
        ffStrbufPutTo(&media->playerPretty, stdout);
    }
    else
    {
        ffPrintFormatString(instance, FF_PLAYER_MODULE_NAME, 0, &instance->config.playerKey, &instance->config.playerFormat, NULL, FF_PLAYER_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->playerPretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->player},
            {FF_FORMAT_ARG_TYPE_STRBUF, &media->busNameShort}
        });
    }
}
