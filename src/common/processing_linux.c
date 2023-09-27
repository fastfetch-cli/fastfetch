#include "fastfetch.h"
#include "common/processing.h"
#include "common/io/io.h"
#include "common/time.h"

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

enum { FF_PIPE_BUFSIZ = 4096 };

static inline void waitpid_wrapper(pid_t* pid)
{
    // remove zombie processes
    if (*pid > 0)
        waitpid(*pid, NULL, 0);
}

const char* ffProcessAppendOutput(FFstrbuf* buffer, char* const argv[], bool useStdErr)
{
    int pipes[2];

    if(pipe(pipes) == -1)
        return "pipe() failed";

    __attribute__((__cleanup__(waitpid_wrapper))) pid_t childPid = fork();
    if(childPid == -1)
        return "fork() failed";

    //Child
    if(childPid == 0)
    {
        dup2(pipes[1], useStdErr ? STDERR_FILENO : STDOUT_FILENO);
        close(pipes[0]);
        close(pipes[1]);
        close(useStdErr ? STDOUT_FILENO : STDERR_FILENO);
        setenv("LANG", "C", 1);
        execvp(argv[0], argv);
        exit(901);
    }

    //Parent
    close(pipes[1]);

    int FF_AUTO_CLOSE_FD childPipeFd = pipes[0];

    int timeout = instance.config.processingTimeout;
    if (timeout >= 0)
        fcntl(childPipeFd, F_SETFL, fcntl(childPipeFd, F_GETFL) | O_NONBLOCK);

    do
    {
        if (timeout >= 0)
        {
            struct pollfd pollfd = { childPipeFd, POLLIN, 0 };
            if (poll(&pollfd, 1, timeout) == 0)
            {
                kill(childPid, SIGTERM);
                return "poll(&pollfd, 1, timeout) timeout";
            }
            else if (pollfd.revents & POLLERR)
            {
                kill(childPid, SIGTERM);
                return "poll(&pollfd, 1, timeout) error";
            }
        }

        char str[FF_PIPE_BUFSIZ];
        while (true)
        {
            ssize_t nRead = read(childPipeFd, str, FF_PIPE_BUFSIZ);
            if (nRead > 0)
                ffStrbufAppendNS(buffer, (uint32_t) nRead, str);
            else if (nRead == 0)
                return NULL;
            else if (nRead < 0)
                break;
        }
    } while (errno == EAGAIN);

    return NULL;
}
