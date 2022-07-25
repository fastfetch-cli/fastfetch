#include "fastfetch.h"
#include "common/io.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void ffProcessAppendStdOut(FFstrbuf* buffer, char* const argv[])
{
    int pipes[2];

    if(pipe(pipes) == -1)
        return;

    pid_t childPid = fork();
    if(childPid == -1)
        return;

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
    waitpid(childPid, NULL, 0);
    ffAppendFDBuffer(pipes[0], buffer);
    close(pipes[0]);
}
