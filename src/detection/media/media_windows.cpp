extern "C" {
#include "media.h"
#include "common/time.h"
#include "common/windows/unicode.h"
#include "common/windows/com.h"
}

#if FF_HAVE_WINRT
    #include <roapi.h>
    #include <robuffer.h>
    #include <winstring.h>
    #include <asyncinfo.h>

    #include <winrt/Windows.ApplicationModel.h>
    #include <winrt/Windows.Foundation.h>
    #include <winrt/Windows.Media.Control.h>
    #include <winrt/Windows.Storage.Streams.h>

    #define FF_BIND_FRONT(method, pobject) std::bind_front(&std::remove_cvref_t<decltype(*(pobject))>::method, (pobject))

using winrt::impl::abi_t;
using winrt::Windows::Foundation::IAsyncOperation;
using winrt::Windows::Foundation::IAsyncOperationWithProgress;

static inline void deleteHstring(HSTRING* pstr) {
    if (*pstr) {
        WindowsDeleteString(*pstr);
    }
}

static inline void ffStrbufSetHstring(FFstrbuf* destination, HSTRING value) {
    uint32_t length;
    const wchar_t* raw = WindowsGetStringRawBuffer(value, &length);
    ffStrbufSetNWS(destination, length, raw);
}

template <typename Interface>
static inline HRESULT ffGetActivationFactory(const wchar_t* className, REFIID iid, Interface** factory) {
    HSTRING_HEADER header;
    HSTRING runtimeClass;
    HRESULT hr = WindowsCreateStringReference(className, (UINT32)::wcslen(className), &header, &runtimeClass);
    if (FAILED(hr)) {
        return hr;
    }

    return RoGetActivationFactory(runtimeClass, iid, reinterpret_cast<void**>(factory));
}

template <typename TargetProjection, typename SourceAbi>
static inline HRESULT ffQueryInterface(SourceAbi* source, abi_t<TargetProjection>** target) {
    return source->QueryInterface(winrt::guid_of<TargetProjection>(), reinterpret_cast<void**>(target));
}

template <typename TOperationAbi, typename TResultAbi>
static HRESULT ffWaitForAsyncOperation(TOperationAbi* operation, TResultAbi** result) {
    IAsyncInfo* FF_AUTO_RELEASE_COM_OBJECT asyncInfo = NULL;
    HRESULT hr = ffQueryInterface<IAsyncInfo>(operation, &asyncInfo);
    if (FAILED(hr)) {
        return hr;
    }

    AsyncStatus status = AsyncStatus::Started;

    for (;;) {
        hr = asyncInfo->get_Status(&status);
        if (FAILED(hr)) {
            return hr;
        }
        if (status == AsyncStatus::Started) {
            ffTimeSleep(0);
        } else {
            break;
        }
    }

    if (status != AsyncStatus::Completed) {
        HRESULT errorCode = E_FAIL;
        asyncInfo->get_ErrorCode(&errorCode);
        return FAILED(errorCode) ? errorCode : E_FAIL;
    }

    return operation->GetResults((void**) result);
}

template <typename TResultProjection, typename TOperation, typename... TArgs>
static HRESULT ffRunAndWait(TOperation&& operation, abi_t<TResultProjection>** result, TArgs&&... args) {
    abi_t<IAsyncOperation<TResultProjection>>* FF_AUTO_RELEASE_COM_OBJECT opResult = NULL;
    HRESULT hr = operation(std::forward<TArgs>(args)..., reinterpret_cast<void**>(&opResult));
    if (FAILED(hr) || !opResult) {
        return hr;
    }

    return ffWaitForAsyncOperation(opResult, result);
}

template <typename TResultProjection, typename TOperation, typename... TArgs>
static HRESULT ffRunAndWait2(TOperation&& operation, abi_t<TResultProjection>** result, TArgs&&... args) {
    *result = NULL;

    abi_t<IAsyncOperationWithProgress<TResultProjection, int32_t>>* FF_AUTO_RELEASE_COM_OBJECT opResult = NULL;
    HRESULT hr = operation(std::forward<TArgs>(args)..., reinterpret_cast<void**>(&opResult));
    if (FAILED(hr) || !opResult) {
        return hr;
    }

    return ffWaitForAsyncOperation(opResult, result);
}

static HRESULT ffSaveThumbnailToTempPath(
    abi_t<winrt::Windows::Storage::Streams::IRandomAccessStreamReference>* thumbnail,
    FFstrbuf* destination) {
    abi_t<winrt::Windows::Storage::Streams::IRandomAccessStreamWithContentType>* FF_AUTO_RELEASE_COM_OBJECT contentStream = NULL;
    HRESULT hr = ffRunAndWait<winrt::Windows::Storage::Streams::IRandomAccessStreamWithContentType>(FF_BIND_FRONT(OpenReadAsync, thumbnail), &contentStream);
    if (FAILED(hr) || !contentStream) {
        return FAILED(hr) ? hr : E_FAIL;
    }

    abi_t<winrt::Windows::Storage::Streams::IRandomAccessStream>* FF_AUTO_RELEASE_COM_OBJECT randomAccessStream = NULL;
    hr = ffQueryInterface<winrt::Windows::Storage::Streams::IRandomAccessStream>(contentStream, &randomAccessStream);
    if (FAILED(hr)) {
        return hr;
    }

    UINT64 size = 0;
    hr = randomAccessStream->get_Size(&size);
    if (FAILED(hr) || size == 0) {
        return FAILED(hr) ? hr : S_FALSE;
    }

    if (size > 0xFFFFFFFFu) {
        return HRESULT_FROM_WIN32(ERROR_FILE_TOO_LARGE);
    }

    abi_t<winrt::Windows::Storage::Streams::IBufferFactory>* FF_AUTO_RELEASE_COM_OBJECT bufferFactory = NULL;
    hr = ffGetActivationFactory(L"Windows.Storage.Streams.Buffer", winrt::guid_of<winrt::Windows::Storage::Streams::IBufferFactory>(), &bufferFactory);
    if (FAILED(hr)) {
        return hr;
    }

    abi_t<winrt::Windows::Storage::Streams::IBuffer>* FF_AUTO_RELEASE_COM_OBJECT buffer = NULL;
    hr = bufferFactory->Create((UINT32) size, reinterpret_cast<void**>(&buffer));
    if (FAILED(hr) || !buffer) {
        return FAILED(hr) ? hr : E_FAIL;
    }

    abi_t<winrt::Windows::Storage::Streams::IInputStream>* FF_AUTO_RELEASE_COM_OBJECT inputStream = NULL;
    hr = ffQueryInterface<winrt::Windows::Storage::Streams::IInputStream>(contentStream, &inputStream);
    if (FAILED(hr)) {
        return hr;
    }

    abi_t<winrt::Windows::Storage::Streams::IBuffer>* FF_AUTO_RELEASE_COM_OBJECT readBuffer = NULL;
    hr = ffRunAndWait2<winrt::Windows::Storage::Streams::IBuffer>(FF_BIND_FRONT(ReadAsync, inputStream), &readBuffer, buffer, (uint32_t) size, static_cast<uint32_t>(winrt::Windows::Storage::Streams::InputStreamOptions::None));
    if (FAILED(hr) || !readBuffer) {
        return FAILED(hr) ? hr : E_FAIL;
    }

    UINT32 length = 0;
    hr = readBuffer->get_Length(&length);
    if (FAILED(hr) || length == 0) {
        return FAILED(hr) ? hr : S_FALSE;
    }

    Windows::Storage::Streams::IBufferByteAccess* FF_AUTO_RELEASE_COM_OBJECT byteAccess = NULL;
    hr = readBuffer->QueryInterface(IID_PPV_ARGS(&byteAccess));
    if (FAILED(hr)) {
        return hr;
    }

    byte* bytes = NULL;
    hr = byteAccess->Buffer(&bytes);
    if (FAILED(hr) || !bytes) {
        return FAILED(hr) ? hr : E_FAIL;
    }

    wchar_t tempDirectory[MAX_PATH];
    DWORD tempLength = GetTempPathW(MAX_PATH, tempDirectory);
    if (tempLength == 0 || tempLength >= MAX_PATH) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    wchar_t tempFilePath[MAX_PATH];
    if (!GetTempFileNameW(tempDirectory, L"fft", 0, tempFilePath)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    HANDLE file = CreateFileW(tempFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        DWORD writeError = GetLastError();
        DeleteFileW(tempFilePath);
        return HRESULT_FROM_WIN32(writeError);
    }

    DWORD written = 0;
    BOOL writtenOk = WriteFile(file, bytes, length, &written, NULL);
    NtClose(file);
    file = NULL;

    if (!writtenOk || written != length) {
        DWORD writeError = GetLastError();
        DeleteFileW(tempFilePath);
        return HRESULT_FROM_WIN32(writtenOk ? ERROR_WRITE_FAULT : writeError);
    }

    ffStrbufSetWS(destination, tempFilePath);
    return S_OK;
}

static const char* getMedia(FFMediaResult* result, bool saveCover) {
    const char* error = ffInitCom();
    if (error) {
        return error;
    }

    do {
        abi_t<winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSessionManagerStatics>* FF_AUTO_RELEASE_COM_OBJECT managerStatics = NULL;
        HRESULT hr = ffGetActivationFactory(L"Windows.Media.Control.GlobalSystemMediaTransportControlsSessionManager", winrt::guid_of<winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSessionManagerStatics>(), &managerStatics);
        if (FAILED(hr) || !managerStatics) {
            error = "winrt: RoGetActivationFactory(GlobalSystemMediaTransportControlsSessionManager) failed";
            break;
        }

        abi_t<winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSessionManager>* FF_AUTO_RELEASE_COM_OBJECT manager = NULL;
        hr = ffRunAndWait<winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSessionManager>(FF_BIND_FRONT(RequestAsync, managerStatics), &manager);
        if (FAILED(hr) || !manager) {
            error = "winrt: RequestAsync().GetResults() failed";
            break;
        }

        abi_t<winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSession>* FF_AUTO_RELEASE_COM_OBJECT session = NULL;
        hr = manager->GetCurrentSession(reinterpret_cast<void**>(&session));

        if (FAILED(hr) || !session) {
            error = "winrt: GetCurrentSession() failed";
            break;
        }

        abi_t<winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSessionMediaProperties>* FF_AUTO_RELEASE_COM_OBJECT mediaProps = NULL;
        hr = ffRunAndWait<winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSessionMediaProperties>(FF_BIND_FRONT(TryGetMediaPropertiesAsync, session), &mediaProps);
        if (FAILED(hr) || !mediaProps) {
            error = "winrt: TryGetMediaPropertiesAsync().GetResults() failed";
            break;
        }

        abi_t<winrt::Windows::Media::Control::IGlobalSystemMediaTransportControlsSessionPlaybackInfo>* FF_AUTO_RELEASE_COM_OBJECT playbackInfo = NULL;
        hr = session->GetPlaybackInfo(reinterpret_cast<void**>(&playbackInfo));
        if (SUCCEEDED(hr) && playbackInfo) {
            int32_t playbackStatusValue = 0;
            if (SUCCEEDED(playbackInfo->get_PlaybackStatus(&playbackStatusValue))) {
                switch (static_cast<winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionPlaybackStatus>(playbackStatusValue)) {
    #define FF_MEDIA_SET_STATUS(status_code)                                                                       \
        case winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionPlaybackStatus::status_code: \
            ffStrbufSetStatic(&result->status, #status_code);                                                      \
            break;
                    FF_MEDIA_SET_STATUS(Closed)
                    FF_MEDIA_SET_STATUS(Opened)
                    FF_MEDIA_SET_STATUS(Changing)
                    FF_MEDIA_SET_STATUS(Stopped)
                    FF_MEDIA_SET_STATUS(Playing)
                    FF_MEDIA_SET_STATUS(Paused)
    #undef FF_MEDIA_SET_STATUS
                }
            }
        }

        FF_A_CLEANUP(deleteHstring) HSTRING playerId = NULL;
        hr = session->get_SourceAppUserModelId(reinterpret_cast<void**>(&playerId));
        if (FAILED(hr)) {
            error = "winrt: get_SourceAppUserModelId() failed";
            break;
        }

        ffStrbufSetHstring(&result->playerId, playerId);

        FF_A_CLEANUP(deleteHstring) HSTRING title = NULL;
        if (SUCCEEDED(mediaProps->get_Title(reinterpret_cast<void**>(&title)))) {
            ffStrbufSetHstring(&result->song, title);
        }

        FF_A_CLEANUP(deleteHstring) HSTRING artist = NULL;
        if (SUCCEEDED(mediaProps->get_Artist(reinterpret_cast<void**>(&artist)))) {
            ffStrbufSetHstring(&result->artist, artist);
        }

        FF_A_CLEANUP(deleteHstring) HSTRING album = NULL;
        if (SUCCEEDED(mediaProps->get_AlbumTitle(reinterpret_cast<void**>(&album)))) {
            ffStrbufSetHstring(&result->album, album);
        }

        abi_t<winrt::Windows::ApplicationModel::IAppInfoStatics>* FF_AUTO_RELEASE_COM_OBJECT appInfoStatics = NULL;
        hr = ffGetActivationFactory(L"Windows.ApplicationModel.AppInfo", winrt::guid_of<winrt::Windows::ApplicationModel::IAppInfoStatics>(), &appInfoStatics);
        if (SUCCEEDED(hr) && appInfoStatics) {
            abi_t<winrt::Windows::ApplicationModel::IAppInfo>* FF_AUTO_RELEASE_COM_OBJECT appInfo = NULL;
            if (SUCCEEDED(appInfoStatics->GetFromAppUserModelId(reinterpret_cast<void*>(playerId), reinterpret_cast<void**>(&appInfo))) && appInfo) {
                abi_t<winrt::Windows::ApplicationModel::IAppDisplayInfo>* FF_AUTO_RELEASE_COM_OBJECT displayInfo = NULL;
                if (SUCCEEDED(appInfo->get_DisplayInfo(reinterpret_cast<void**>(&displayInfo))) && displayInfo) {
                    FF_A_CLEANUP(deleteHstring) HSTRING displayName = NULL;
                    if (SUCCEEDED(displayInfo->get_DisplayName(reinterpret_cast<void**>(&displayName)))) {
                        ffStrbufSetHstring(&result->player, displayName);
                    }
                }
            }
        }

        if (result->player.length == 0) {
            ffStrbufSet(&result->player, &result->playerId);
            if (ffStrbufEndsWithIgnCaseS(&result->player, ".exe")) {
                ffStrbufSubstrBefore(&result->player, result->player.length - 4);
            }
        }

        if (saveCover) {
            abi_t<winrt::Windows::Storage::Streams::IRandomAccessStreamReference>* FF_AUTO_RELEASE_COM_OBJECT thumbnail = NULL;
            hr = mediaProps->get_Thumbnail(reinterpret_cast<void**>(&thumbnail));
            if (SUCCEEDED(hr) && thumbnail) {
                if (SUCCEEDED(ffSaveThumbnailToTempPath(thumbnail, &result->cover)) && result->cover.length > 0) {
                    result->removeCoverAfterUse = true;
                }
            }
        }
    } while (false);

    return error;
}
#else
static const char* getMedia(FFMediaResult* media, bool saveCover) {
    FF_UNUSED(media, saveCover);
    return "Fastfetch is not compiled with WinRT support";
}
#endif // FF_HAVE_WINRT

extern "C" void ffDetectMediaImpl(FFMediaResult* media, bool saveCover) {
    const char* error = getMedia(media, saveCover);
    ffStrbufAppendS(&media->error, error);
}
