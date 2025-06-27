#include "fastfetch.h"
#include "detection/media/media.h"
#include "common/processing.h"

#import <Foundation/Foundation.h>

@interface MRContentItem : NSObject <NSCopying>
@property (nonatomic, readonly, copy) NSDictionary<NSString*, NSString*>* nowPlayingInfo;
@end

@interface MRClient : NSObject <NSCopying, NSSecureCoding>
@property (nonatomic, copy) NSString* bundleIdentifier;
@property (nonatomic, copy) NSString* displayName;
@end

@interface MRPlayerPath : NSObject <NSCopying, NSSecureCoding>
@property (nonatomic, copy) MRClient* client;
@end

@interface MRNowPlayingRequest
+ (bool)localIsPlaying;
+ (MRContentItem*)localNowPlayingItem;
+ (MRPlayerPath*)localNowPlayingPlayerPath;
@end

static const char* getMediaByMediaRemote(FFMediaResult* result)
{
    if (!NSClassFromString(@"MRNowPlayingRequest"))
        return "MediaRemote framework is not available";
    if (!MRNowPlayingRequest.localNowPlayingItem)
        return "No media found";
    ffStrbufSetStatic(&result->status, MRNowPlayingRequest.localIsPlaying ? "Playing" : "Paused");

    NSDictionary<NSString*, NSString*>* infoDict = MRNowPlayingRequest.localNowPlayingItem.nowPlayingInfo;
    ffStrbufSetS(&result->song, infoDict[@"kMRMediaRemoteNowPlayingInfoTitle"].UTF8String);
    ffStrbufSetS(&result->artist, infoDict[@"kMRMediaRemoteNowPlayingInfoArtist"].UTF8String);
    ffStrbufSetS(&result->album, infoDict[@"kMRMediaRemoteNowPlayingInfoAlbum"].UTF8String);

    MRClient* bundleObj = MRNowPlayingRequest.localNowPlayingPlayerPath.client;
    ffStrbufSetS(&result->playerId, bundleObj.bundleIdentifier.UTF8String);
    ffStrbufSetS(&result->player, bundleObj.displayName.UTF8String);
    return NULL;
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
    if (error) ffStrbufAppendS(&media->error, error);
}
