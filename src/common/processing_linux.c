#include "fastfetch.h"
#include "common/processing.h"
#include "common/io/io.h"
#include "common/time.h"

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>

const char* ffProcessAppendOutput(FFstrbuf* buffer, char* const argv[], bool useStdErr)
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
        dup2(pipes[1], useStdErr ? STDERR_FILENO : STDOUT_FILENO);
        close(pipes[0]);
        close(pipes[1]);
        close(useStdErr ? STDOUT_FILENO : STDERR_FILENO);
        execvp(argv[0], argv);
        exit(901);
    }

    //Parent
    close(pipes[1]);

    int FF_AUTO_CLOSE_FD childPipeFd = pipes[0];
    if (instance.config.processingTimeout >= 0)
    {
        struct pollfd pollfd = { childPipeFd, POLLIN, 0 };
        if (poll(&pollfd, 1, (int) instance.config.processingTimeout) == 0)
        {
            kill(childPid, SIGTERM);
            return "poll(&pollfd, 1, (int) instance.config.processingTimeout) timeout";
        }
        else if (pollfd.revents & POLLERR)
        {
            kill(childPid, SIGTERM);
            return "poll(&pollfd, 1, (int) instance.config.processingTimeout) error";
        }
        else if (pollfd.revents & POLLHUP)
        {
            return "Child process closed its end (nothing to read)";
        }
    }

    // Note that we only know we have something to read here
    // However the child process may still block later
    if(!ffAppendFDBuffer(childPipeFd, buffer))
        return "ffAppendFDBuffer(childPipeFd, buffer) failed";

    return NULL;
}
