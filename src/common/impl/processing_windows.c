#include "fastfetch.h"
#include "common/processing.h"
#include "common/io.h"
#include "common/windows/unicode.h"
#include "common/windows/nt.h"

#include <stdalign.h>
#include <windows.h>
#include <ntstatus.h>

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
    swprintf(pipeName, ARRAY_SIZE(pipeName), L"\\\\.\\pipe\\FASTFETCH-%u-%u", instance.state.platform.pid, ++pidCounter);

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

    NtClose(hChildPipeWrite);
    if(!success)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
            return "command not found";
        return "CreateProcessA() failed";
    }

    NtClose(piProcInfo.hThread); // we don't need the thread handle
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
    FF_AUTO_CLOSE_FD HANDLE hReadEvent = NULL;
    handle->pid = INVALID_HANDLE_VALUE;
    handle->pipeRead = INVALID_HANDLE_VALUE;

    if (timeout >= 0 && !NT_SUCCESS(NtCreateEvent(&hReadEvent, EVENT_ALL_ACCESS, NULL, SynchronizationEvent, FALSE)))
        return "NtCreateEvent() failed";

    char str[FF_PIPE_BUFSIZ];
    uint32_t nRead = 0;
    IO_STATUS_BLOCK iosb = {};
    do
    {
        NTSTATUS status = NtReadFile(
            hChildPipeRead,
            hReadEvent,
            NULL,
            NULL,
            &iosb,
            str,
            (ULONG) sizeof(str),
            NULL,
            NULL
        );
        if (status == STATUS_PENDING)
        {
            switch (NtWaitForSingleObject(hReadEvent, TRUE, &(LARGE_INTEGER) { .QuadPart = (int64_t) timeout * -10000 }))
            {
            case STATUS_WAIT_0:
                status = iosb.Status;
                break;

            case STATUS_TIMEOUT:
                CancelIo(hChildPipeRead);
                TerminateProcess(hProcess, 1);
                return "NtReadFile(hChildPipeRead) timed out";

            default:
                CancelIo(hChildPipeRead);
                TerminateProcess(hProcess, 1);
                return "WaitForSingleObject(hReadEvent) failed";
            }
        }

        if (status == STATUS_PIPE_BROKEN || status == STATUS_END_OF_FILE)
            goto exit;

        if (!NT_SUCCESS(status))
        {
            CancelIo(hChildPipeRead);
            TerminateProcess(hProcess, 1);
            return "NtReadFile(hChildPipeRead) failed";
        }

        nRead = (uint32_t) iosb.Information;
        ffStrbufAppendNS(buffer, nRead, str);
    } while (nRead > 0);

exit:
    {
        PROCESS_BASIC_INFORMATION info = {};
        ULONG size;
        if(NT_SUCCESS(NtQueryInformationProcess(hProcess, ProcessBasicInformation, &info, sizeof(info), &size)))
        {
            assert(size == sizeof(info));
            if (info.ExitStatus != STILL_ACTIVE && info.ExitStatus != 0)
                return "Child process exited with an error";
        }
        else
            return "NtQueryInformationProcess(ProcessBasicInformation) failed";
    }

    return NULL;
}

bool ffProcessGetInfoWindows(uint32_t pid, uint32_t* ppid, FFstrbuf* pname, FFstrbuf* exe, const char** exeName, FFstrbuf* exePath, bool* gui)
{
    FF_AUTO_CLOSE_FD HANDLE hProcess = pid == 0
        ? NtCurrentProcess()
        : OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);

    if (hProcess == NULL)
        return false;

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
        // TODO: It's possible to query the command line with `NtQueryInformationProcess(60/*ProcessCommandLineInformation*/)` since Windows 8.1

        alignas(UNICODE_STRING) uint8_t buffer[4096];
        ULONG size;
        if(NT_SUCCESS(NtQueryInformationProcess(hProcess, ProcessImageFileNameWin32, &buffer, sizeof(buffer), &size)))
        {
            UNICODE_STRING* imagePath = (UNICODE_STRING*)buffer;
            ffStrbufSetNWS(exe, imagePath->Length / sizeof(wchar_t), imagePath->Buffer);

            if (exePath) ffStrbufSet(exePath, exe);

            if (pname && exeName)
            {
                *exeName = exe->chars + ffStrbufLastIndexC(exe, '\\') + 1;
                ffStrbufSetS(pname, *exeName);
            }
        }
        else
            return false;
    }

    if (gui)
    {
        SECTION_IMAGE_INFORMATION info = {};
        ULONG size;
        if(NT_SUCCESS(NtQueryInformationProcess(hProcess, ProcessImageInformation, &info, sizeof(info), &size)))
        {
            assert(size == sizeof(info));
            *gui = info.SubSystemType == IMAGE_SUBSYSTEM_WINDOWS_GUI;
        }
        else
            return false;
    }

    return true;
}
