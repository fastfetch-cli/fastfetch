#include "fastfetch.h"
#include "common/processing.h"
#include "common/apple/cf_helpers.h"
#include "common/time.h"
#include "detection/media/media.h"

#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#import <CoreServices/CoreServices.h>

// https://github.com/andrewwiik/iOS-Blocks/blob/master/Widgets/Music/MediaRemote.h
[[clang::weak_import]] extern void MRMediaRemoteGetNowPlayingInfo(dispatch_queue_t dispatcher, void (^callback)(_Nullable CFDictionaryRef info));
[[clang::weak_import]] extern void MRMediaRemoteGetNowPlayingApplicationIsPlaying(dispatch_queue_t queue, void (^callback)(BOOL playing));
[[clang::weak_import]] extern void MRMediaRemoteGetNowPlayingApplicationDisplayID(dispatch_queue_t queue, void (^callback)(_Nullable CFStringRef displayID));
[[clang::weak_import]] extern void MRMediaRemoteGetNowPlayingApplicationDisplayName(int unknown, dispatch_queue_t queue, void (^callback)(_Nullable CFStringRef name));

static uint32_t getTrueElapsedTime(CFDictionaryRef info) {
    double elapsedTime;
    if (ffCfDictGetDouble(info, CFSTR("kMRMediaRemoteNowPlayingInfoElapsedTime"), &elapsedTime) != nullptr) {
        return 0;
    }

    elapsedTime *= 1000;

    double playbackRate;
    uint64_t timestampEpoch;
    if (ffCfDictGetDouble(info, CFSTR("kMRMediaRemoteNowPlayingInfoPlaybackRate"), &playbackRate) == nullptr &&
        ffCfDictGetDateAsEpoch(info, CFSTR("kMRMediaRemoteNowPlayingInfoTimestamp"), &timestampEpoch) == nullptr) {
        uint64_t timeDiff = ffTimeGetNow() - timestampEpoch;
        elapsedTime += (double) timeDiff * playbackRate;
    }

    return (uint32_t) elapsedTime;
}

static const char* getMediaByMediaRemote(FFMediaResult* result, bool saveCover) {
#define FF_TEST_FN_EXISTENCE(fn) \
    if (!fn) return "MediaRemote function " #fn " is not available"
    FF_TEST_FN_EXISTENCE(MRMediaRemoteGetNowPlayingInfo);
    FF_TEST_FN_EXISTENCE(MRMediaRemoteGetNowPlayingApplicationIsPlaying);
#undef FF_TEST_FN_EXISTENCE

    dispatch_group_t group = dispatch_group_create();
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0);

    dispatch_group_enter(group);
    __block const char* error = nullptr;
    MRMediaRemoteGetNowPlayingInfo(queue, ^(_Nullable CFDictionaryRef info) {
        if (info != nil) {
            ffCfDictGetString(info, CFSTR("kMRMediaRemoteNowPlayingInfoTitle"), &result->song);
            ffCfDictGetString(info, CFSTR("kMRMediaRemoteNowPlayingInfoArtist"), &result->artist);
            ffCfDictGetString(info, CFSTR("kMRMediaRemoteNowPlayingInfoAlbum"), &result->album);
            double value;
            if (ffCfDictGetDouble(info, CFSTR("kMRMediaRemoteNowPlayingInfoDuration"), &value) == nullptr) {
                result->length = (uint32_t) (value * 1000);
                result->position = getTrueElapsedTime(info);
            }

            if (saveCover) {
                NSData* artworkData = (__bridge NSData*) CFDictionaryGetValue(info, CFSTR("kMRMediaRemoteNowPlayingInfoArtworkData"));
                if (artworkData) {
                    CFStringRef mime = (CFStringRef) CFDictionaryGetValue(info, CFSTR("kMRMediaRemoteNowPlayingInfoArtworkMIMEType"));
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                    FF_CFTYPE_AUTO_RELEASE CFStringRef uti = UTTypeCreatePreferredIdentifierForTag(kUTTagClassMIMEType, mime, nullptr);
                    FF_CFTYPE_AUTO_RELEASE CFStringRef ext = UTTypeCopyPreferredTagWithClass(uti, kUTTagClassFilenameExtension);
#pragma clang diagnostic pop
                    NSString* tmpDir = NSTemporaryDirectory();
                    NSString* uuid = NSUUID.UUID.UUIDString;
                    NSString* path = [tmpDir stringByAppendingPathComponent:[NSString stringWithFormat:@"ff_%@.%@", uuid, ext ? (__bridge NSString*) ext : @"img"]];
                    if ([artworkData writeToFile:path atomically:NO])
                        ffStrbufSetS(&result->cover, path.UTF8String);
                }
            }
        } else
            error = "MRMediaRemoteGetNowPlayingInfo() failed";

        dispatch_group_leave(group);
    });

    dispatch_group_enter(group);
    MRMediaRemoteGetNowPlayingApplicationIsPlaying(queue, ^(BOOL playing) {
        ffStrbufSetStatic(&result->status, playing ? "Playing" : "Paused");
        dispatch_group_leave(group);
    });

    if (MRMediaRemoteGetNowPlayingApplicationDisplayID) {
        dispatch_group_enter(group);
        MRMediaRemoteGetNowPlayingApplicationDisplayID(queue, ^(_Nullable CFStringRef displayID) {
            ffCfStrGetString(displayID, &result->playerId);
            dispatch_group_leave(group);
        });
    }

    if (MRMediaRemoteGetNowPlayingApplicationDisplayName) {
        dispatch_group_enter(group);
        MRMediaRemoteGetNowPlayingApplicationDisplayName(0, queue, ^(_Nullable CFStringRef name) {
            ffCfStrGetString(name, &result->player);
            dispatch_group_leave(group);
        });
    }

    dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
    // Don't dispatch_release because we are using ARC

    if (result->song.length > 0) {
        return nullptr;
    }

    return error;
}

void ffDetectMediaImpl(FFMediaResult* media, bool saveCover) {
    const char* error = getMediaByMediaRemote(media, saveCover);

    if (error) {
        ffStrbufAppendS(&media->error, error);
    } else {
        if (media->player.length == 0 && media->playerId.length > 0) {
            ffStrbufSet(&media->player, &media->playerId);
            if (ffStrbufStartsWithIgnCaseS(&media->player, "com.")) {
                ffStrbufSubstrAfter(&media->player, strlen("com.") - 1);
            }
            ffStrbufReplaceAllC(&media->player, '.', ' ');
        }
        // The cover is a temporary file that this process created, so it is ours to delete. This used
        // to be flagged only in the branch above, which meant the file was left behind on every run
        // whose player name was already known — that is, almost all of them.
        if (media->cover.length > 0) {
            media->removeCoverAfterUse = true;
        }
    }
}
