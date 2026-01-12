#include "detection/command/command.h"
#include "common/processing.h"
#include "common/FFstrbuf.h"

typedef struct FFCommandResultBundle
{
    FFProcessHandle handle;
    const char* error;
} FFCommandResultBundle;

// FIFO, non-thread-safe list of running commands
static FFlist commandQueue = {
    .elementSize = sizeof(FFCommandResultBundle),
};

static const char* spawnProcess(FFCommandOptions* options, FFProcessHandle* handle)
{
    if (options->text.length == 0)
        return "No command text specified";

    return ffProcessSpawn(options->param.length ? (char* const[]){
        options->shell.chars,
        options->param.chars,
        options->text.chars,
        NULL
    } : (char* const[]){
        options->shell.chars,
        options->text.chars,
        NULL
    }, options->useStdErr, handle);
}

bool ffPrepareCommand(FFCommandOptions* options)
{
    if (!options->parallel) return false;

    FFCommandResultBundle* bundle = ffListAdd(&commandQueue);
    bundle->error = spawnProcess(options, &bundle->handle);

    return true;
}

const char* ffDetectCommand(FFCommandOptions* options, FFstrbuf* result)
{
    FFCommandResultBundle bundle = {};
    if (!options->parallel)
        bundle.error = spawnProcess(options, &bundle.handle);
    else if (!ffListShift(&commandQueue, &bundle))
        return "[BUG] command queue is empty";

    if (bundle.error)
        return bundle.error;

    bundle.error = ffProcessReadOutput(&bundle.handle, result);
    if (bundle.error)
        return bundle.error;

    ffStrbufTrimRightSpace(result);
    return NULL;
}
