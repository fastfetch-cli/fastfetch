#include "fastfetch.h"
#include "common/processing.h"

#include <Windows.h>

const char* ffProcessAppendStdOut(FFstrbuf* buffer, char* const argv[])
{
    SECURITY_ATTRIBUTES saAttr = {
        .nLength = sizeof(SECURITY_ATTRIBUTES),
        .lpSecurityDescriptor = NULL,
        .bInheritHandle = TRUE,
    };

    HANDLE hChildPipeRead, hChildPipeWrite;
    if (!CreatePipe(&hChildPipeRead, &hChildPipeWrite, &saAttr, 0))
        return "CreatePipe() failed";

    if (!SetHandleInformation(hChildPipeRead, HANDLE_FLAG_INHERIT, 0))
        return "SetHandleInformation(hChildPipeRead) failed";

    PROCESS_INFORMATION piProcInfo = {0};
    STARTUPINFOA siStartInfo = {
        .cb = sizeof(siStartInfo),
        .dwFlags = STARTF_USESTDHANDLES,
        .hStdOutput = hChildPipeWrite,
    };

    BOOL success;

    {
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
    {
        CloseHandle(hChildPipeRead);
        return "CreateProcessA() failed";
    }

    char str[1024];
    DWORD nRead;
    while(ReadFile(hChildPipeRead, str, sizeof(str), &nRead, NULL) && nRead > 0)
        ffStrbufAppendNS(buffer, nRead, str);

    CloseHandle(hChildPipeRead);
    return NULL;
}

const char* ffProcessAppendStdErr(FFstrbuf* buffer, char* const argv[])
{
    SECURITY_ATTRIBUTES saAttr = {
        .nLength = sizeof(SECURITY_ATTRIBUTES),
        .lpSecurityDescriptor = NULL,
        .bInheritHandle = TRUE,
    };

    HANDLE hChildPipeRead, hChildPipeWrite;
    if (!CreatePipe(&hChildPipeRead, &hChildPipeWrite, &saAttr, 0))
        return "CreatePipe() failed";

    if (!SetHandleInformation(hChildPipeRead, HANDLE_FLAG_INHERIT, 0))
        return "SetHandleInformation(hChildPipeRead) failed";

    PROCESS_INFORMATION piProcInfo = {0};
    STARTUPINFOA siStartInfo = {
        .cb = sizeof(siStartInfo),
        .dwFlags = STARTF_USESTDHANDLES,
        .hStdError = hChildPipeWrite,
    };

    BOOL success;

    {
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
    {
        CloseHandle(hChildPipeRead);
        return "CreateProcessA() failed";
    }

    char str[1024];
    DWORD nRead;
    while(ReadFile(hChildPipeRead, str, sizeof(str), &nRead, NULL) && nRead > 0)
        ffStrbufAppendNS(buffer, nRead, str);

    CloseHandle(hChildPipeRead);
    return NULL;
}
