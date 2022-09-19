#include "fastfetch.h"
#include "detection/media/media.h"
#include "common/library.h"
#include "util/apple/cfdict_helpers.h"

#import <pthread.h>
#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>

void MRMediaRemoteGetNowPlayingInfo(dispatch_queue_t dispatcher, void(^callback)(_Nullable CFDictionaryRef info));

const char* getMedia(FFMediaResult* result)
{
    ffStrbufInit(&result->busNameShort);
    ffStrbufInit(&result->player);
    ffStrbufInit(&result->song);
    ffStrbufInit(&result->artist);
    ffStrbufInit(&result->album);
    ffStrbufInit(&result->url);

    FFstrbuf fake;
    ffStrbufInitA(&fake, 0);//MediaRemote is a macOS builtin framework thus its path should not change

    FF_LIBRARY_LOAD(
        MediaRemote,
        fake,
        "dlopen MediaRemote failed",
        "/System/Library/PrivateFrameworks/MediaRemote.framework/MediaRemote",
        1);
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
    return NULL;
}

const FFMediaResult* ffDetectMedia(FFinstance* instance)
{
    FF_UNUSED(instance)
    static FFMediaResult result;
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool init = false;

    if(!init)
    {
        pthread_mutex_lock(&mutex);
        if(!init)
        {
            result.error = getMedia(&result);
            init = true;
        }
        pthread_mutex_unlock(&mutex);
    }

    return &result;
}
