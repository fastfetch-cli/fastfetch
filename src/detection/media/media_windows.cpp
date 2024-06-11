extern "C"
{
    #include "media.h"
    #include "util/windows/unicode.h"
}

#include <winrt/Windows.Foundation.h>
#include <winrt/windows.Media.Control.h>
#include <winrt/Windows.Media.Playback.h>

extern "C"
{
static const char* getMedia(FFMediaResult* media)
{
    using namespace winrt::Windows::Media::Control;
    auto manager = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
    if (!manager)
        return "GlobalSystemMediaTransportControlsSessionManager::RequestAsync() failed";

    auto session = manager.GetCurrentSession();
    if (!session)
        return "GetCurrentSession() failed";

    auto mediaProps = session
        .TryGetMediaPropertiesAsync()
        .get();
    if (!mediaProps)
        return "TryGetMediaPropertiesAsync() failed";

    if (auto playbackInfo = session.GetPlaybackInfo())
    {
        switch (playbackInfo.PlaybackStatus())
        {
#define FF_MEDIA_SET_STATUS(status_code) \
case GlobalSystemMediaTransportControlsSessionPlaybackStatus::status_code: ffStrbufSetStatic(&media->status, #status_code); break
            FF_MEDIA_SET_STATUS(Closed);
            FF_MEDIA_SET_STATUS(Opened);
            FF_MEDIA_SET_STATUS(Changing);
            FF_MEDIA_SET_STATUS(Stopped);
            FF_MEDIA_SET_STATUS(Playing);
            FF_MEDIA_SET_STATUS(Paused);
#undef FF_MEDIA_SET_STATUS
        }
    }

    ffStrbufSetNWS(&media->playerId, session.SourceAppUserModelId().size(), session.SourceAppUserModelId().data());
    ffStrbufSet(&media->player, &media->playerId);
    if (ffStrbufEndsWithIgnCaseS(&media->player, ".exe"))
        ffStrbufSubstrBefore(&media->player, media->player.length - 4);
    else
        ffStrbufSubstrAfterFirstC(&media->player, '!'); // UWP ID

    ffStrbufSetNWS(&media->song, mediaProps.Title().size(), mediaProps.Title().data());
    ffStrbufSetNWS(&media->artist, mediaProps.Artist().size(), mediaProps.Artist().data());
    ffStrbufSetNWS(&media->album, mediaProps.AlbumTitle().size(), mediaProps.AlbumTitle().data());
    return NULL;
}

void ffDetectMediaImpl(FFMediaResult* media)
{
    const char* error = getMedia(media);
    ffStrbufAppendS(&media->error, error);
}

}
