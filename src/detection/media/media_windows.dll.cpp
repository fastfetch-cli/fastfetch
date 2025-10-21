#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Storage.h>
#include <wchar.h>
#include <windows.h>

extern "C"
{
#include "media_windows.dll.h"

const char* ffWinrtDetectMedia(FFWinrtMediaResult* result)
{
    // C++/WinRT requires Windows 8.1+ and C++ runtime (std::string, exceptions and other stuff)
    // Make it a separate dll in order not to break Windows 7 support
    using namespace winrt::Windows::Media::Control;
    using namespace winrt::Windows::ApplicationModel;
    using namespace winrt::Windows::Storage;
    using namespace winrt::Windows::Storage::Streams;

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
        result->playerId[FF_MEDIA_WIN_RESULT_BUFLEN - 1] = L'\0';
        ::wcsncpy(result->song, mediaProps.Title().data(), FF_MEDIA_WIN_RESULT_BUFLEN);
        result->song[FF_MEDIA_WIN_RESULT_BUFLEN - 1] = L'\0';
        ::wcsncpy(result->artist, mediaProps.Artist().data(), FF_MEDIA_WIN_RESULT_BUFLEN);
        result->artist[FF_MEDIA_WIN_RESULT_BUFLEN - 1] = L'\0';
        ::wcsncpy(result->album, mediaProps.AlbumTitle().data(), FF_MEDIA_WIN_RESULT_BUFLEN);
        result->album[FF_MEDIA_WIN_RESULT_BUFLEN - 1] = L'\0';
        try
        {
            // Only works for UWP apps
            ::wcsncpy(result->playerName, AppInfo::GetFromAppUserModelId(session.SourceAppUserModelId()).DisplayInfo().DisplayName().data(), FF_MEDIA_WIN_RESULT_BUFLEN);
            result->playerName[FF_MEDIA_WIN_RESULT_BUFLEN - 1] = L'\0';
        } catch (...) { }

        if (auto thumbRef = mediaProps.Thumbnail())
        {
            try
            {
                if (auto stream = thumbRef.OpenReadAsync().get())
                {
                    if (stream.Size() > 0)
                    {
                        Buffer buffer(static_cast<uint32_t>(stream.Size()));
                        stream.ReadAsync(buffer, buffer.Capacity(), InputStreamOptions::None).get();

                        wchar_t tempPath[MAX_PATH];
                        if (GetTempPathW(MAX_PATH, tempPath) > 0)
                        {
                            auto tempFolder = StorageFolder::GetFolderFromPathAsync(tempPath).get();
                            auto tempFile = tempFolder.CreateFileAsync(L"ff_thumb.img", CreationCollisionOption::GenerateUniqueName).get();
                            FileIO::WriteBufferAsync(tempFile, buffer).get();

                            ::wcsncpy(result->cover, tempFile.Path().data(), FF_MEDIA_WIN_RESULT_BUFLEN);
                            result->cover[FF_MEDIA_WIN_RESULT_BUFLEN - 1] = L'\0';
                        }
                    }
                }
            }
            catch (...)
            {
                // Ignore thumbnail errors
            }
        }

        return NULL;
    }
    catch (...)
    {
        return "A C++ exception is thrown";
    }
}

} // extern "C"
