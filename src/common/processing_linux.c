#include "fastfetch.h"
#include "common/processing.h"
#include "common/io/io.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

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
    if(waitpid(childPid, &status, 0) < 0)
        return "waitpid(childPid, &status, 0) failed";

    if (!WIFEXITED(status))
        return "WIFEXITED(status) == false";

    if(WEXITSTATUS(status) == 901)
        return "WEXITSTATUS(status) == 901 ( execvp failed )";

    if(!ffAppendFDBuffer(childPipeFd, buffer))
        return "ffAppendFDBuffer(childPipeFd, buffer) failed";

    return NULL;
}
