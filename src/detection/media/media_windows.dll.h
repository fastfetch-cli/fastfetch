#pragma once

#define FF_MEDIA_WIN_RESULT_BUFLEN 256

typedef struct FFWinrtMediaResult
{
    wchar_t playerId[FF_MEDIA_WIN_RESULT_BUFLEN];
    wchar_t song[FF_MEDIA_WIN_RESULT_BUFLEN];
    wchar_t artist[FF_MEDIA_WIN_RESULT_BUFLEN];
    wchar_t album[FF_MEDIA_WIN_RESULT_BUFLEN];
    const char* status;
} FFWinrtMediaResult;

__attribute__((__dllexport__))
const char* ffWinrtDetectMedia(FFWinrtMediaResult* result);
