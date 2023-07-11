#include "fastfetch.h"
#include "common/processing.h"
#include "common/io/io.h"

#include <Windows.h>

enum { FF_PIPE_BUFSIZ = 4096 };

const char* ffProcessAppendOutput(FFstrbuf* buffer, char* const argv[], bool useStdErr)
{
    int timeout = instance.config.processingTimeout;

    FF_AUTO_CLOSE_FD HANDLE hChildPipeRead = CreateNamedPipeW(
        L"\\\\.\\pipe\\LOCAL\\",
        PIPE_ACCESS_INBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE | (timeout < 0 ? 0 : FILE_FLAG_OVERLAPPED),
        0,
        1,
        FF_PIPE_BUFSIZ,
        FF_PIPE_BUFSIZ,
        0,
        NULL
    );
    if (hChildPipeRead == INVALID_HANDLE_VALUE)
        return "CreateNamedPipeW(L\"\\\\.\\pipe\\LOCAL\\\") failed";

    HANDLE hChildPipeWrite = CreateFileW(
        L"\\\\.\\pipe\\LOCAL\\",
        GENERIC_WRITE,
        0,
        &(SECURITY_ATTRIBUTES){
            .nLength = sizeof(SECURITY_ATTRIBUTES),
            .lpSecurityDescriptor = NULL,
            .bInheritHandle = TRUE,
        },
        OPEN_EXISTING,
        0,
        NULL
    );
    if (hChildPipeWrite == INVALID_HANDLE_VALUE)
        return "CreateFileW(L\"\\\\.\\pipe\\LOCAL\\\") failed";

    PROCESS_INFORMATION piProcInfo = {0};

    BOOL success;

    {
        STARTUPINFOA siStartInfo = {
            .cb = sizeof(siStartInfo),
            .dwFlags = STARTF_USESTDHANDLES,
        };
        if (useStdErr)
            siStartInfo.hStdError = hChildPipeWrite;
        else
            siStartInfo.hStdOutput = hChildPipeWrite;

        FF_STRBUF_AUTO_DESTROY cmdline = ffStrbufCreateF("\"%s\"", argv[0]);
        for(char* const* parg = &argv[1]; *parg; ++parg)
        {
            ffStrbufAppendC(&cmdline, ' ');
            ffStrbufAppendS(&cmdline, *parg);
        }

        success = CreateProcessA(
            NULL,          // application name
            cmdline.chars, // command line
            NULL,          // process security attributes
            NULL,          // primary thread security attributes
            TRUE,          // handles are inherited
            0,             // creation flags
            NULL,          // use parent's environment
            NULL,          // use parent's current directory
            &siStartInfo,  // STARTUPINFO pointer
            &piProcInfo    // receives PROCESS_INFORMATION
        );
    }

    CloseHandle(hChildPipeWrite);
    if(!success)
        return "CreateProcessA() failed";

    char str[FF_PIPE_BUFSIZ];
    DWORD nRead = 0;
    OVERLAPPED overlapped = {};
    // ReadFile always completes synchronously if the pipe is not created with FILE_FLAG_OVERLAPPED
    if (!ReadFile(hChildPipeRead, str, sizeof(str), &nRead, &overlapped))
    {
        if (!GetOverlappedResultEx(hChildPipeRead, &overlapped, &nRead, (DWORD) timeout, TRUE))
        {
            if (GetLastError() == ERROR_BROKEN_PIPE)
                return "Child process closed its end (nothing to read)";
            CancelIo(hChildPipeRead);
            TerminateProcess(piProcInfo.hProcess, 1);
            return "GetOverlappedResultEx(hChildPipeRead) failed or timeout";
        }
    }
    while (nRead > 0)
    {
        ffStrbufAppendNS(buffer, nRead, str);
        if (!ReadFile(hChildPipeRead, str, sizeof(str), &nRead, &overlapped))
        {
            if (!GetOverlappedResult(hChildPipeRead, &overlapped, &nRead, TRUE))
            {
                if (GetLastError() == ERROR_BROKEN_PIPE)
                    return NULL;
                CancelIo(hChildPipeRead);
                return "GetOverlappedResult(hChildPipeRead) failed";
            }
        }
    }

    return NULL;
}
