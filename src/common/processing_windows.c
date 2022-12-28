#include "fastfetch.h"
#include "common/processing.h"

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

//We can't use this native version yet because we still use POSIX path ( eg /usr/bin/fish ) at a lot of places.
const char* ffProcessAppendStdOut(FFstrbuf* buffer, char* const argv[])
{
    SECURITY_ATTRIBUTES saAttr = {
        .nLength = sizeof(SECURITY_ATTRIBUTES),
        .lpSecurityDescriptor = NULL,
        .bInheritHandle = TRUE,
    };

    HANDLE hChildStdoutRead, hChildStdoutWrite;
    if (!CreatePipe(&hChildStdoutRead, &hChildStdoutWrite, &saAttr, 0))
        return "CreatePipe() failed";

    if (!SetHandleInformation(hChildStdoutRead, HANDLE_FLAG_INHERIT, 0))
        return "SetHandleInformation(hChildStdoutRead) failed";

    PROCESS_INFORMATION piProcInfo = {0};
    STARTUPINFOA siStartInfo = {
        .cb = sizeof(siStartInfo),
        .dwFlags = STARTF_USESTDHANDLES,
        .hStdOutput = hChildStdoutWrite,
    };

    BOOL success;

    {
        FF_STRBUF_AUTO_DESTROY cmdline;
        ffStrbufInitF(&cmdline, "\"%s\"", argv[0]);
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

    CloseHandle(hChildStdoutWrite);
    if(!success)
    {
        CloseHandle(hChildStdoutRead);
        return "CreateProcessA() failed";
    }

    char str[1024];
    DWORD nRead;
    while(ReadFile(hChildStdoutRead, str, sizeof(str), &nRead, NULL) && nRead > 0)
        ffStrbufAppendNS(buffer, nRead, str);

    CloseHandle(hChildStdoutRead);
    return NULL;
}
