#include "common/processing.h"
#include "fastfetch.h"

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ProcessReadContext {
    FFProcessHandle handle;
    FFstrbuf output;
    const char* error;
} ProcessReadContext;

static void* readOutput(void* data) {
    ProcessReadContext* context = data;
    context->error = ffProcessReadOutput(&context->handle, &context->output);
    return nullptr;
}

static pid_t parseDescendantPid(const FFstrbuf* output) {
    if (!output->chars || output->length == 0) {
        return -1;
    }

    char* end = nullptr;
    long pid = strtol(output->chars, &end, 10);
    if (end == output->chars || pid <= 0) {
        return -1;
    }

    return (pid_t) pid;
}

static void stopDescendant(const FFstrbuf* output) {
    pid_t pid = parseDescendantPid(output);
    if (pid > 0) {
        kill(pid, SIGKILL);
    }
}

int main(void) {
    // The direct shell exits immediately, while its background sleep keeps the output pipe open.
    char* const successArgv[] = {
        "/bin/sh",
        "-c",
        "sleep 30 & printf '%s\\n' \"$!\"",
        nullptr,
    };
    char* const failureArgv[] = {
        "/bin/sh",
        "-c",
        "sleep 30 & printf '%s\\n' \"$!\"; exit 127",
        nullptr,
    };
    const char* const expectedErrors[] = { nullptr, "command not found" };

    instance.config.general.processingTimeout = 1000;

    ProcessReadContext contexts[2] = {};
    size_t spawned = 0;
    bool spawnFailed = false;
    for (; spawned < 2;) {
        contexts[spawned].output = ffStrbufCreate();
        char* const* argv = spawned == 0 ? successArgv : failureArgv;
        const char* error = ffProcessSpawn(argv, false, ffGetNullFD(), &contexts[spawned].handle);
        if (error) {
            fprintf(stderr, "ffProcessSpawn failed: %s\n", error);
            ffStrbufDestroy(&contexts[spawned].output);
            spawnFailed = true;
            break;
        }
        spawned++;
    }

    pthread_t threads[2];
    size_t started = 0;
    if (!spawnFailed) {
        for (; started < 2; started++) {
            if (pthread_create(&threads[started], nullptr, readOutput, &contexts[started]) != 0) {
                fprintf(stderr, "pthread_create failed\n");
                break;
            }
        }
    }

    for (size_t i = started; i < spawned; i++) {
        contexts[i].error = ffProcessReadOutput(&contexts[i].handle, &contexts[i].output);
    }
    for (size_t i = 0; i < started; i++) {
        pthread_join(threads[i], nullptr);
    }

    bool passed = !spawnFailed && started == 2;
    for (size_t i = 0; i < spawned; i++) {
        stopDescendant(&contexts[i].output);
        bool errorMatches = expectedErrors[i] == nullptr
            ? contexts[i].error == nullptr
            : contexts[i].error && strcmp(contexts[i].error, expectedErrors[i]) == 0;
        if (!errorMatches) {
            if (contexts[i].error) {
                fprintf(stderr, "ffProcessReadOutput returned an unexpected error: %s\n", contexts[i].error);
            } else {
                fprintf(stderr, "ffProcessReadOutput returned success unexpectedly\n");
            }
            passed = false;
        }
        if (parseDescendantPid(&contexts[i].output) <= 0) {
            fprintf(stderr, "child PID was not captured from command output\n");
            passed = false;
        }
        ffStrbufDestroy(&contexts[i].output);
    }

    instance.config.general.processingTimeout = 100;
    FFstrbuf timeoutOutput = ffStrbufCreate();
    FFProcessHandle timeoutHandle;
    char* const timeoutArgv[] = { "/bin/sleep", "30", nullptr };
    const char* timeoutError = ffProcessSpawn(timeoutArgv, false, ffGetNullFD(), &timeoutHandle);
    if (timeoutError == nullptr) {
        timeoutError = ffProcessReadOutput(&timeoutHandle, &timeoutOutput);
    }
    if (!timeoutError
        || strcmp(timeoutError, "poll(&pollfd, 1, timeout) timeout (try increasing --processing-timeout)") != 0) {
        fprintf(stderr, "processing timeout was not reported\n");
        passed = false;
    }
    ffStrbufDestroy(&timeoutOutput);

    if (!passed) {
        return 1;
    }

    puts("All processing tests passed!");
    return 0;
}
