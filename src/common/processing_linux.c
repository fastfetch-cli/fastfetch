#include "fastfetch.h"
#include "common/processing.h"
#include "common/io/io.h"
#include "common/time.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>

#ifdef __linux__
    #include <sys/syscall.h>
    #include <sys/poll.h>
    #include <errno.h>
#endif

#define FF_WAIT_TIMEOUT 1000

int waitpid_timeout(pid_t pid, int* status)
{
    if (FF_WAIT_TIMEOUT <= 0)
        return waitpid(pid, status, 0);

    #ifdef __linux__

    FF_AUTO_CLOSE_FD int pidfd = (int) syscall(SYS_pidfd_open, pid, 0);
    if (pidfd >= 0)
    {
        int res = poll(&(struct pollfd) { .events = POLLIN, .fd = pidfd }, 1, FF_WAIT_TIMEOUT);
        if (res > 0)
            return (int) waitpid(pid, status, WNOHANG);
        else if (res == 0)
        {
            kill(pid, SIGTERM);
            return -ETIME;
        }
        return -1;
    }

    #endif

    uint64_t start = ffTimeGetTick();
    while (true)
    {
        int res = (int) waitpid(pid, status, WNOHANG);
        if (res != 0)
            return res;
        if (ffTimeGetTick() - start < FF_WAIT_TIMEOUT)
            ffTimeSleep(FF_WAIT_TIMEOUT / 10);
        else
        {
            kill(pid, SIGTERM);
            return -ETIME;
        }
    }
}

const char* ffProcessAppendStdOut(FFstrbuf* buffer, char* const argv[])
{
    int pipes[2];

    if(pipe(pipes) == -1)
        return "pipe() failed";

    pid_t childPid = fork();
    if(childPid == -1)
        return "fork() failed";

    //Child
    if(childPid == 0)
    {
        dup2(pipes[1], STDOUT_FILENO);
        close(pipes[0]);
        close(pipes[1]);
        close(STDERR_FILENO);
        execvp(argv[0], argv);
        exit(901);
    }

    //Parent
    close(pipes[1]);

    int FF_AUTO_CLOSE_FD childPipeFd = pipes[0];
    int status = -1;
    if(waitpid_timeout(childPid, &status) < 0)
        return "waitpid(childPid, &status) failed";

    if (!WIFEXITED(status))
        return "WIFEXITED(status) == false";

    if(WEXITSTATUS(status) == 901)
        return "WEXITSTATUS(status) == 901 ( execvp failed )";

    if(!ffAppendFDBuffer(childPipeFd, buffer))
        return "ffAppendFDBuffer(childPipeFd, buffer) failed";

    return NULL;
}

const char* ffProcessAppendStdErr(FFstrbuf* buffer, char* const argv[])
{
    int pipes[2];

    if(pipe(pipes) == -1)
        return "pipe() failed";

    pid_t childPid = fork();
    if(childPid == -1)
        return "fork() failed";

    //Child
    if(childPid == 0)
    {
        dup2(pipes[1], STDERR_FILENO);
        close(pipes[0]);
        close(pipes[1]);
        close(STDOUT_FILENO);
        execvp(argv[0], argv);
        exit(901);
    }

    //Parent
    close(pipes[1]);

    int FF_AUTO_CLOSE_FD childPipeFd = pipes[0];
    int status = -1;
    if(waitpid_timeout(childPid, &status) < 0)
        return "waitpid(childPid, &status) failed";

    if (!WIFEXITED(status))
        return "WIFEXITED(status) == false";

    if(WEXITSTATUS(status) == 901)
        return "WEXITSTATUS(status) == 901 ( execvp failed )";

    if(!ffAppendFDBuffer(childPipeFd, buffer))
        return "ffAppendFDBuffer(childPipeFd, buffer) failed";

    return NULL;
}
