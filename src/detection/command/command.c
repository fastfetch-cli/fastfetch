#include "detection/command/command.h"
#include "common/processing.h"
#include "common/FFstrbuf.h"

typedef struct FFCommandResultBundle {
    FFProcessHandle handle;
    const char* error;
} FFCommandResultBundle;

// FIFO, non-thread-safe list of running commands
static FFlist commandQueue;

static const char* spawnProcess(FFCommandOptions* options, FFProcessHandle* handle) {
    if (options->text.length == 0) {
        return "No command text specified";
    }

    return ffProcessSpawn(options->param.length ? (char* const[]) {
                                                      options->shell.chars,
                                                      options->param.chars,
                                                      options->text.chars,
                                                      nullptr }
                                                : (char* const[]) { options->shell.chars, options->text.chars, nullptr },
        options->useStdErr,
        FF_PROCESS_INHERIT_STDIN,
        handle);
}

bool ffPrepareCommand(FFCommandOptions* options) {
    if (!options->parallel) {
        return false;
    }

    FFCommandResultBundle* bundle = FF_LIST_ADD(FFCommandResultBundle, commandQueue);
    bundle->error = spawnProcess(options, &bundle->handle);

    return true;
}

const char* ffDetectCommand(FFCommandOptions* options, FFstrbuf* result) {
    FFCommandResultBundle bundle = {};
    if (!options->parallel) {
        bundle.error = spawnProcess(options, &bundle.handle);
    } else if (!FF_LIST_SHIFT(commandQueue, &bundle)) {
        // The module was printed without having been prepared first, so there is no process to collect.
        // That is the expected outcome for a module gated on `condition.succeeded`: whether it is
        // printed at all is only known in the print pass, so the prepare pass skips it (see
        // `printJsonConfig`). In every other case the queue and the printed modules must line up.
        return "Module was not prepared, so it cannot run in parallel. Modules gated on "
               "`condition.succeeded` are never prepared, which is the expected cause; "
               "anything else is a bug in fastfetch, please report it";
    }

    if (bundle.error) {
        return bundle.error;
    }

    bundle.error = ffProcessReadOutput(&bundle.handle, result);
    if (bundle.error) {
        return bundle.error;
    }

    ffStrbufTrimRightSpace(result);
    return nullptr;
}
