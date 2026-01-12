#include "fastfetch.h"
#include "common/processing.h"
#include "common/io.h"

#include <windows.h>
#include <ntstatus.h>
#include <winternl.h>

enum { FF_PIPE_BUFSIZ = 8192 };

static void argvToCmdline(char* const argv[], FFstrbuf* result)
{
    // From https://gist.github.com/jin-x/cdd641d98887524b091fb1f82a68717d

    FF_STRBUF_AUTO_DESTROY temp = ffStrbufCreate();
    for (int i = 0; argv[i] != NULL; i++)
    {
        ffStrbufSetS(&temp, argv[i]);
        // Add slash (\) before double quotes (") and duplicate slashes before it
        for (
            uint32_t pos = ffStrbufFirstIndexC(&temp, '"'), cnt;
            pos != temp.length;
            pos = ffStrbufNextIndexC(&temp, pos + cnt * 2, '"')
        ) {
            cnt = 1;
            while (pos > 0 && temp.chars[pos - 1] == '\\') { ++cnt, --pos; }
            ffStrbufInsertNC(&temp, pos, cnt, '\\');
        }

        // Add quotes around string if whitespace chars are present (with slash duplicating at the end of string)
        if (ffStrbufFirstIndexS(&temp, " \t") != temp.length)
        {
            uint32_t pos = temp.length;
            uint32_t cnt = 0;
            while (pos > 0 && temp.chars[pos - 1] == '\\') { ++cnt, --pos; }
            if (cnt > 0) ffStrbufAppendNC(&temp, cnt, '\\');
            ffStrbufPrependC(&temp, '"');
            ffStrbufAppendC(&temp, '"');
        }

        // Add space delimiter
        if (i > 0) ffStrbufAppendC(result, ' ');
        ffStrbufAppend(result, &temp);
        ffStrbufClear(&temp);
    }
}

const char* ffProcessSpawn(char* const argv[], bool useStdErr, FFProcessHandle* outHandle)
{
    const int32_t timeout = instance.config.general.processingTimeout;

    wchar_t pipeName[32];
    static unsigned pidCounter = 0;
    swprintf(pipeName, ARRAY_SIZE(pipeName), L"\\\\.\\pipe\\FASTFETCH-%u-%u", GetCurrentProcessId(), ++pidCounter);

    FF_AUTO_CLOSE_FD HANDLE hChildPipeRead = CreateNamedPipeW(
        pipeName,
        PIPE_ACCESS_INBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE | (timeout < 0 ? 0 : FILE_FLAG_OVERLAPPED),
        0,
        1,
        FF_PIPE_BUFSIZ,
        FF_PIPE_BUFSIZ,
        0,
        NULL
    );
    if (hChildPipeRead == INVALID_HANDLE_VALUE)
        return "CreateNamedPipeW(L\"\\\\.\\pipe\\FASTFETCH-$(PID)\") failed";

    HANDLE hChildPipeWrite = CreateFileW(
        pipeName,
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
        return "CreateFileW(L\"\\\\.\\pipe\\FASTFETCH-$(PID)\") failed";

    PROCESS_INFORMATION piProcInfo = {};
    STARTUPINFOA siStartInfo = {
        .cb = sizeof(siStartInfo),
        .dwFlags = STARTF_USESTDHANDLES,
    };
    if (useStdErr)
    {
        siStartInfo.hStdOutput = ffGetNullFD();
        siStartInfo.hStdError = hChildPipeWrite;
    }
    else
    {
        siStartInfo.hStdOutput = hChildPipeWrite;
        siStartInfo.hStdError = ffGetNullFD();
    }

    FF_STRBUF_AUTO_DESTROY cmdline = ffStrbufCreate();
    argvToCmdline(argv, &cmdline);

    BOOL success = CreateProcessA(
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

    CloseHandle(hChildPipeWrite);
    if(!success)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
            return "command not found";
        return "CreateProcessA() failed";
    }

    CloseHandle(piProcInfo.hThread); // we don't need the thread handle
    outHandle->pid   = piProcInfo.hProcess;
    outHandle->pipeRead  = hChildPipeRead;
    hChildPipeRead = INVALID_HANDLE_VALUE; // ownership transferred, don't close it

    return NULL;
}

const char* ffProcessReadOutput(FFProcessHandle* handle, FFstrbuf* buffer)
{
    assert(handle->pipeRead != INVALID_HANDLE_VALUE);
    assert(handle->pid != INVALID_HANDLE_VALUE);

    int32_t timeout = instance.config.general.processingTimeout;
    FF_AUTO_CLOSE_FD HANDLE hProcess = handle->pid;
    FF_AUTO_CLOSE_FD HANDLE hChildPipeRead = handle->pipeRead;
    handle->pid = INVALID_HANDLE_VALUE;
    handle->pipeRead = INVALID_HANDLE_VALUE;

    char str[FF_PIPE_BUFSIZ];
    DWORD nRead = 0;
    OVERLAPPED overlapped = {};
    // ReadFile always completes synchronously if the pipe is not created with FILE_FLAG_OVERLAPPED
    do
    {
        if (!ReadFile(hChildPipeRead, str, sizeof(str), &nRead, &overlapped))
        {
            switch (GetLastError())
            {
            case ERROR_IO_PENDING:
                #if !FF_WIN7_COMPAT
                if (!GetOverlappedResultEx(hChildPipeRead, &overlapped, &nRead, timeout < 0 ? INFINITE : (DWORD) timeout, FALSE))
                #else
                // To support Windows 7
                if (timeout >= 0 && WaitForSingleObject(hChildPipeRead, (DWORD) timeout) != WAIT_OBJECT_0)
                {
                    CancelIo(hChildPipeRead);
                    TerminateProcess(hProcess, 1);
                    return "WaitForSingleObject(hChildPipeRead) failed or timeout (try increasing --processing-timeout)";
                }

                if (!GetOverlappedResult(hChildPipeRead, &overlapped, &nRead, FALSE))
                #endif
                {
                    if (GetLastError() == ERROR_BROKEN_PIPE)
                        return NULL;

                    CancelIo(hChildPipeRead);
                    TerminateProcess(hProcess, 1);
                    return "GetOverlappedResult"
                        #if !FF_WIN7_COMPAT
                        "Ex"
                        #endif
                        "(hChildPipeRead) failed";
                }
                break;

            case ERROR_BROKEN_PIPE:
                goto exit;

            default:
                CancelIo(hChildPipeRead);
                TerminateProcess(hProcess, 1);
                return "ReadFile(hChildPipeRead) failed";
            }
        }
        ffStrbufAppendNS(buffer, nRead, str);
    } while (nRead > 0);

exit:
    {
        DWORD exitCode = 0;
        if (GetExitCodeProcess(hProcess, &exitCode) && exitCode != STILL_ACTIVE && exitCode != 0)
            return "Child process exited with an error";
    }

    return NULL;
}

bool ffProcessGetInfoWindows(uint32_t pid, uint32_t* ppid, FFstrbuf* pname, FFstrbuf* exe, const char** exeName, FFstrbuf* exePath, bool* gui)
{
    FF_AUTO_CLOSE_FD HANDLE hProcess = pid == 0
        ? GetCurrentProcess()
        : OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);

    if (hProcess == NULL)
        return false;

    if (gui)
        *gui = GetGuiResources(hProcess, GR_GDIOBJECTS) > 0;

    if(ppid)
    {
        PROCESS_BASIC_INFORMATION info = {};
        ULONG size;
        if(NT_SUCCESS(NtQueryInformationProcess(hProcess, ProcessBasicInformation, &info, sizeof(info), &size)))
        {
            assert(size == sizeof(info));
            *ppid = (uint32_t)info.InheritedFromUniqueProcessId;
        }
        else
            return false;
    }
    if(exe)
    {
        DWORD bufSize = exe->allocated;
        if(QueryFullProcessImageNameA(hProcess, 0, exe->chars, &bufSize))
        {
            // We use full path here
            // Querying command line of remote processes in Windows requires either WMI or ReadProcessMemory
            exe->length = bufSize;
            if (exePath) ffStrbufSet(exePath, exe);
        }
        else
            return false;
    }
    if(pname && exeName)
    {
        *exeName = exe->chars + ffStrbufLastIndexC(exe, '\\') + 1;
        ffStrbufSetS(pname, *exeName);
    }

    return true;
}
