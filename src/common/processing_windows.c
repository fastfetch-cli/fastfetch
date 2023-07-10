#include "fastfetch.h"
#include "common/processing.h"
#include "common/io/io.h"

#include <Windows.h>

const char* ffProcessAppendOutput(FFstrbuf* buffer, char* const argv[], bool useStdErr)
{
    SECURITY_ATTRIBUTES saAttr = {
        .nLength = sizeof(SECURITY_ATTRIBUTES),
        .lpSecurityDescriptor = NULL,
        .bInheritHandle = TRUE,
    };

    FF_AUTO_CLOSE_FD HANDLE hChildPipeRead = NULL;
    HANDLE hChildPipeWrite = NULL;
    if (!CreatePipe(&hChildPipeRead, &hChildPipeWrite, &saAttr, 0))
        return "CreatePipe() failed";

    if (!SetHandleInformation(hChildPipeRead, HANDLE_FLAG_INHERIT, 0))
        return "SetHandleInformation(hChildPipeRead) failed";

    PROCESS_INFORMATION piProcInfo = {0};
    STARTUPINFOA siStartInfo = {
        .cb = sizeof(siStartInfo),
        .dwFlags = STARTF_USESTDHANDLES,
    };
    if (useStdErr)
        siStartInfo.hStdError = hChildPipeWrite;
    else
        siStartInfo.hStdOutput = hChildPipeWrite;

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
        return "CreateProcessA() failed";

    if (FF_WAIT_TIMEOUT > 0)
    {
        DWORD ret = WaitForSingleObjectEx(piProcInfo.hProcess, FF_WAIT_TIMEOUT, TRUE);
        if (ret == WAIT_TIMEOUT)
        {
            TerminateProcess(piProcInfo.hProcess, 1);
            return "Waiting process timeout";
        }
    }

    char str[1024];
    DWORD nRead;
    while(ReadFile(hChildPipeRead, str, sizeof(str), &nRead, NULL) && nRead > 0)
        ffStrbufAppendNS(buffer, nRead, str);

    return NULL;
}
