#include "fastfetch.h"

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

void ffProcessGetStdOut(FFstrbuf* buffer, char* const argv[])
{
    int pipes[2];

    if(pipe(pipes) == -1)
        return;

    pid_t childPid = fork();
    if(childPid == -1)
        return;

    if(childPid == 0)
    {
        dup2(pipes[1], STDOUT_FILENO);
        close(pipes[0]);
        close(pipes[1]);
        execvp(argv[0], argv);
        exit(901);
    }
    else
    {
        close(pipes[1]);
        waitpid(childPid, NULL, 0);
        ffAppendFDContent(pipes[0], buffer);
        close(pipes[0]);
    }
}
