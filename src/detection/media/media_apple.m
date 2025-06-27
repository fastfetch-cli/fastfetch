#include "fastfetch.h"
#include "detection/media/media.h"
#include "common/processing.h"
#include "util/apple/cf_helpers.h"

#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>

// https://github.com/andrewwiik/iOS-Blocks/blob/master/Widgets/Music/MediaRemote.h
extern void MRMediaRemoteGetNowPlayingInfo(dispatch_queue_t dispatcher, void(^callback)(_Nullable CFDictionaryRef info)) __attribute__((weak_import));
extern void MRMediaRemoteGetNowPlayingApplicationIsPlaying(dispatch_queue_t queue, void (^callback)(BOOL playing)) __attribute__((weak_import));
extern void MRMediaRemoteGetNowPlayingApplicationDisplayID(dispatch_queue_t queue, void (^callback)(_Nullable CFStringRef displayID)) __attribute__((weak_import));
extern void MRMediaRemoteGetNowPlayingApplicationDisplayName(int unknown, dispatch_queue_t queue, void (^callback)(_Nullable CFStringRef name)) __attribute__((weak_import));

static const char* getMediaByMediaRemote(FFMediaResult* result)
{
    #define FF_TEST_FN_EXISTANCE(fn) if (!fn) return "MediaRemote function " #fn " is not available"
    FF_TEST_FN_EXISTANCE(MRMediaRemoteGetNowPlayingInfo);
    FF_TEST_FN_EXISTANCE(MRMediaRemoteGetNowPlayingApplicationIsPlaying);
    #undef FF_TEST_FN_EXISTANCE

    dispatch_group_t group = dispatch_group_create();
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0);

    dispatch_group_enter(group);
    __block const char* error = NULL;
    MRMediaRemoteGetNowPlayingInfo(queue, ^(_Nullable CFDictionaryRef info) {
        if(info != nil)
        {
            ffCfDictGetString(info, CFSTR("kMRMediaRemoteNowPlayingInfoTitle"), &result->song);
            ffCfDictGetString(info, CFSTR("kMRMediaRemoteNowPlayingInfoArtist"), &result->artist);
            ffCfDictGetString(info, CFSTR("kMRMediaRemoteNowPlayingInfoAlbum"), &result->album);
        }
        else
            error = "MRMediaRemoteGetNowPlayingInfo() failed";

        dispatch_group_leave(group);
    });

    dispatch_group_enter(group);
    MRMediaRemoteGetNowPlayingApplicationIsPlaying(queue, ^(BOOL playing) {
        ffStrbufSetStatic(&result->status, playing ? "Playing" : "Paused");
        dispatch_group_leave(group);
    });

    if (MRMediaRemoteGetNowPlayingApplicationDisplayID)
    {
        dispatch_group_enter(group);
        MRMediaRemoteGetNowPlayingApplicationDisplayID(queue, ^(_Nullable CFStringRef displayID) {
            ffCfStrGetString(displayID, &result->playerId);
            dispatch_group_leave(group);
        });
    }

    if (MRMediaRemoteGetNowPlayingApplicationDisplayName)
    {
        dispatch_group_enter(group);
        MRMediaRemoteGetNowPlayingApplicationDisplayName(0, queue, ^(_Nullable CFStringRef name) {
            ffCfStrGetString(name, &result->player);
            dispatch_group_leave(group);
        });
    }

    dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
    // Don't dispatch_release because we are using ARC

    if(result->song.length > 0)
        return NULL;

    return error;
}

__attribute__((visibility("default"), used))
int ffPrintMediaByMediaRemote(void)
{
    FFMediaResult media = {
        .status = ffStrbufCreate(),
        .song = ffStrbufCreate(),
        .artist = ffStrbufCreate(),
        .album = ffStrbufCreate(),
        .playerId = ffStrbufCreate(),
        .player = ffStrbufCreate(),
    };
    if (getMediaByMediaRemote(&media) != NULL)
        return 1;
    ffStrbufPutTo(&media.status, stdout);
    ffStrbufPutTo(&media.song, stdout);
    ffStrbufPutTo(&media.artist, stdout);
    ffStrbufPutTo(&media.album, stdout);
    ffStrbufPutTo(&media.playerId, stdout);
    ffStrbufPutTo(&media.player, stdout);
    ffStrbufDestroy(&media.status);
    ffStrbufDestroy(&media.song);
    ffStrbufDestroy(&media.artist);
    ffStrbufDestroy(&media.album);
    ffStrbufDestroy(&media.playerId);
    ffStrbufDestroy(&media.player);
    return 0;
}

static const char* getMediaByAuthorizedProcess(FFMediaResult* result)
{
    // #1737
    FF_STRBUF_AUTO_DESTROY script = ffStrbufCreateF("import ctypes;ctypes.CDLL('%s').ffPrintMediaByMediaRemote()", instance.state.platform.exePath.chars);
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    const char* error = ffProcessAppendStdOut(&buffer, (char* const[]) {
        "/usr/bin/python3", // Must be signed by Apple. Homebrew python doesn't work
        "-c",
        script.chars,
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
        error = getMediaByAuthorizedProcess(media);
    else
        error = getMediaByMediaRemote(media);
    if (error)
        ffStrbufAppendS(&media->error, error);
    else if (media->player.length == 0 && media->playerId.length > 0)
    {
        ffStrbufSet(&media->player, &media->playerId);
        if (ffStrbufStartsWithIgnCaseS(&media->player, "com."))
            ffStrbufSubstrAfter(&media->player, strlen("com.") - 1);
        ffStrbufReplaceAllC(&media->player, '.', ' ');
    }
}
