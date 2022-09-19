#include "fastfetch.h"
#include "detection/media/media.h"
#include "common/library.h"
#include "util/apple/cfdict_helpers.h"

#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>

void MRMediaRemoteGetNowPlayingInfo(dispatch_queue_t dispatcher, void(^callback)(_Nullable CFDictionaryRef info));

static const char* getMedia(FFMediaResult* result)
{
    FF_LIBRARY_LOAD(MediaRemote, NULL, "dlopen MediaRemote failed", "/System/Library/PrivateFrameworks/MediaRemote.framework/MediaRemote", -1);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(MediaRemote, MRMediaRemoteGetNowPlayingInfo);

    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    ffMRMediaRemoteGetNowPlayingInfo(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^(_Nullable CFDictionaryRef info) {
        if(info != nil) {
            ffCfDictGetString(info, CFSTR("kMRMediaRemoteNowPlayingInfoTitle"), &result->song);
            ffCfDictGetString(info, CFSTR("kMRMediaRemoteNowPlayingInfoArtist"), &result->artist);
            ffCfDictGetString(info, CFSTR("kMRMediaRemoteNowPlayingInfoAlbum"), &result->album);
        }
        dispatch_semaphore_signal(semaphore);
    });
    dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);

    if(result->song.length > 0)
        return NULL;

    return "MediaRemote failed";
}

void ffDetectMediaImpl(const FFinstance* instance, FFMediaResult* media)
{
    FF_UNUSED(instance)
    const char* error = getMedia(media);
    ffStrbufAppendS(&media->error, error);

    //TODO: proper detection
    //I already set it here, because the player module expects it to be set if the error is not set
    if(error == NULL)
        ffStrbufAppendS(&media->player, "Media Player");
}
