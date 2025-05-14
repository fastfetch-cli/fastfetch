#include "fastfetch.h"
#include "detection/media/media.h"
#include "common/library.h"
#include "common/processing.h"
#include "util/apple/cf_helpers.h"
#include "util/apple/osascript.h"
#include "fastfetch_datatext.h"

#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>

extern void MRMediaRemoteGetNowPlayingInfo(dispatch_queue_t dispatcher, void(^callback)(_Nullable CFDictionaryRef info)) __attribute__((weak_import));
extern void MRMediaRemoteGetNowPlayingClient(dispatch_queue_t dispatcher, void (^callback)(_Nullable id clientObj)) __attribute__((weak_import));
extern CFStringRef MRNowPlayingClientGetBundleIdentifier(id clientObj) __attribute__((weak_import));
extern CFStringRef MRNowPlayingClientGetParentAppBundleIdentifier(id clientObj) __attribute__((weak_import));
void MRMediaRemoteGetNowPlayingApplicationIsPlaying(dispatch_queue_t queue, void (^callback)(BOOL playing)) __attribute__((weak_import));

static const char* getMediaByMediaRemote(FFMediaResult* result)
{
    #define FF_TEST_FN_EXISTANCE(fn) if (!fn) return "MediaRemote function " #fn " is not available"
    FF_TEST_FN_EXISTANCE(MRMediaRemoteGetNowPlayingInfo);
    FF_TEST_FN_EXISTANCE(MRMediaRemoteGetNowPlayingClient);
    FF_TEST_FN_EXISTANCE(MRNowPlayingClientGetBundleIdentifier);
    FF_TEST_FN_EXISTANCE(MRNowPlayingClientGetParentAppBundleIdentifier);
    FF_TEST_FN_EXISTANCE(MRMediaRemoteGetNowPlayingApplicationIsPlaying);
    #undef FF_TEST_FN_EXISTANCE

    dispatch_group_t group = dispatch_group_create();
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0);

    dispatch_group_enter(group);
    MRMediaRemoteGetNowPlayingApplicationIsPlaying(queue, ^(BOOL playing) {
        ffStrbufSetStatic(&result->status, playing ? "Playing" : "Paused");
        dispatch_group_leave(group);
    });

    dispatch_group_enter(group);
    __block const char* error = NULL;
    MRMediaRemoteGetNowPlayingInfo(queue, ^(_Nullable CFDictionaryRef info) {
        if(info != nil)
        {
            error = ffCfDictGetString(info, CFSTR("kMRMediaRemoteNowPlayingInfoTitle"), &result->song);
            if(!error)
            {
                ffCfDictGetString(info, CFSTR("kMRMediaRemoteNowPlayingInfoArtist"), &result->artist);
                ffCfDictGetString(info, CFSTR("kMRMediaRemoteNowPlayingInfoAlbum"), &result->album);
            }
        }
        else
            error = "MRMediaRemoteGetNowPlayingInfo() failed";

        dispatch_group_leave(group);
    });

    dispatch_group_enter(group);
    MRMediaRemoteGetNowPlayingClient(queue, ^(_Nullable id clientObj) {
        if (clientObj != nil)
        {
            CFStringRef identifier = MRNowPlayingClientGetBundleIdentifier(clientObj);
            if (identifier == nil)
                identifier = MRNowPlayingClientGetParentAppBundleIdentifier(clientObj);
            if (identifier != nil)
                ffCfStrGetString(identifier, &result->playerId);
        }
        dispatch_group_leave(group);
    });

    dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
    // Don't dispatch_release because we are using ARC

    if(result->playerId.length > 0)
    {
        char buf[128];
        snprintf(buf, ARRAY_SIZE(buf), "name of app id \"%s\"", result->playerId.chars);
        ffOsascript(buf, &result->player);
    }

    if(result->song.length > 0)
        return NULL;

    return error;
}

static const char* getMediaByOsascript(FFMediaResult* result)
{
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    // The script must be executed by `/usr/bin/osascript`. `ffOsascript` doesn't work here
    const char* error = ffProcessAppendStdOut(&buffer, (char* const[]) {
        "/usr/bin/osascript",
        "-e",
        FASTFETCH_DATATEXT_NOWPLAYING,
        nil
    });
    if (error) return error;
    if (buffer.length == 0) return "No media found";

    // status\ntitle\nartist\nalbum\nbundleName\nappName
    FFstrbuf* const varList[] = { &result->status, &result->song, &result->artist, &result->album, &result->playerId, &result->player };
    char* line = NULL;
    size_t len = 0;
    for (uint32_t i = 0; i < ARRAY_SIZE(varList) && ffStrbufGetline(&line, &len, &buffer); ++i)
        ffStrbufSetS(varList[i], line);
    return NULL;
}

void ffDetectMediaImpl(FFMediaResult* media)
{
    const char* error;
    if (@available(macOS 15.4, *))
        error = getMediaByOsascript(media);
    else
        error = getMediaByMediaRemote(media);
    if (error) ffStrbufAppendS(&media->error, error);
}
