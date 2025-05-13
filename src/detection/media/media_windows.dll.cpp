#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.ApplicationModel.h>
#include <wchar.h>

extern "C"
{
#include "media_windows.dll.h"

const char* ffWinrtDetectMedia(FFWinrtMediaResult* result)
{
    // C++/WinRT requires Windows 8.1+ and C++ runtime (std::string, exceptions and other stuff)
    // Make it a separate dll in order not to break Windows 7 support
    using namespace winrt::Windows::Media::Control;
    using namespace winrt::Windows::ApplicationModel;

    try
    {
        auto manager = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
        if (!manager)
            return "winrt: GlobalSystemMediaTransportControlsSessionManager::RequestAsync() failed";

        auto session = manager.GetCurrentSession();
        if (!session)
            return "winrt: GetCurrentSession() failed";

        auto mediaProps = session
            .TryGetMediaPropertiesAsync()
            .get();
        if (!mediaProps)
            return "winrt: TryGetMediaPropertiesAsync() failed";

        if (auto playbackInfo = session.GetPlaybackInfo())
        {
            switch (playbackInfo.PlaybackStatus())
            {
            #define FF_MEDIA_SET_STATUS(status_code) \
    case GlobalSystemMediaTransportControlsSessionPlaybackStatus::status_code: result->status = #status_code; break;
                FF_MEDIA_SET_STATUS(Closed)
                FF_MEDIA_SET_STATUS(Opened)
                FF_MEDIA_SET_STATUS(Changing)
                FF_MEDIA_SET_STATUS(Stopped)
                FF_MEDIA_SET_STATUS(Playing)
                FF_MEDIA_SET_STATUS(Paused)
            #undef FF_MEDIA_SET_STATUS
            }
        }

        ::wcsncpy(result->playerId, session.SourceAppUserModelId().data(), FF_MEDIA_WIN_RESULT_BUFLEN);
        ::wcsncpy(result->song, mediaProps.Title().data(), FF_MEDIA_WIN_RESULT_BUFLEN);
        ::wcsncpy(result->artist, mediaProps.Artist().data(), FF_MEDIA_WIN_RESULT_BUFLEN);
        ::wcsncpy(result->album, mediaProps.AlbumTitle().data(), FF_MEDIA_WIN_RESULT_BUFLEN);
        try
        {
            // Only works for UWP apps
            ::wcsncpy(result->playerName, AppInfo::GetFromAppUserModelId(session.SourceAppUserModelId()).DisplayInfo().DisplayName().data(), FF_MEDIA_WIN_RESULT_BUFLEN);
        } catch (...) { }

        return NULL;
    }
    catch (...)
    {
        return "A C++ exception is thrown";
    }
}

} // extern "C"
