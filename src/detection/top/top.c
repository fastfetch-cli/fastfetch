#include "top.h"

#include "common/time.h"

static FFlist first;
static double startTick;
static FFTopTypes preparedTypes;

void ffPrepareTopProcesses(FFTopTypes showTypes) {
    if (startTick != 0) {
        return; // Already prepared
    }

    if ((showTypes & (FF_TOP_TYPE_CPU | FF_TOP_TYPE_DISK)) == 0) {
        return; // Memory usage is instantaneous; no baseline snapshot is needed
    }

    // Within one module `showTypes` cannot change between this call and `ffDetectTopProcesses`:
    // `ffPrepareCommandOption` and `parseStructureCommand` both build the options through
    // `initStructureModuleOptions`, which merges the module object from the JSON config, so the
    // baseline always matches what the second snapshot collects.
    preparedTypes = showTypes;
    ffListInit(&first);
    startTick = ffTimeGetTick();
    ffTopGetProcessSnapshot(&first, showTypes);
}

// clang-format off
static int compareCpuResults(const FFTopProcessResult* a, const FFTopProcessResult* b) {
    if (a->cpuPercent < b->cpuPercent) return 1;
    if (a->cpuPercent > b->cpuPercent) return -1;
    return (int) (a->pid - b->pid);
}

static int compareMemoryResults(const FFTopProcessResult* a, const FFTopProcessResult* b) {
    if (a->memBytes < b->memBytes) return 1;
    if (a->memBytes > b->memBytes) return -1;
    return (int) (a->pid - b->pid);
}

static int compareDiskReadResults(const FFTopProcessResult* a, const FFTopProcessResult* b) {
    if (a->bytesRead < b->bytesRead) return 1;
    if (a->bytesRead > b->bytesRead) return -1;
    return (int) (a->pid - b->pid);
}

static int compareDiskWriteResults(const FFTopProcessResult* a, const FFTopProcessResult* b) {
    if (a->bytesWritten < b->bytesWritten) return 1;
    if (a->bytesWritten > b->bytesWritten) return -1;
    return (int) (a->pid - b->pid);
}

static int compareStartTimeResults(const FFTopProcessResult* a, const FFTopProcessResult* b) {
    if (a->startTime < b->startTime) return 1;
    if (a->startTime > b->startTime) return -1;
    return (int) (a->pid - b->pid);
}

static int compareThreadsResults(const FFTopProcessResult* a, const FFTopProcessResult* b) {
    if (a->threads < b->threads) return 1;
    if (a->threads > b->threads) return -1;
    return (int) (a->pid - b->pid);
}
// clang-format on

const char* ffDetectTopProcesses(FFTopOptions* options, FFlist* result) {
    ffListClear(result);
    if (options->nProcesses == 0) {
        return nullptr;
    }

    // A baseline collected for a different set of counters is answered here rather than in
    // ffPrepareTopProcesses, which has no way to report an error to the caller. The module that
    // triggered the mismatch is the one that sees it; see ffPrepareTopProcesses.
    if (options->showTypes != preparedTypes && (options->showTypes & (FF_TOP_TYPE_CPU | FF_TOP_TYPE_DISK)) != 0) {
        return "`top` modules with different `showTypes` cannot share a run";
    }

    // Memory usage and thread count are instantaneous; when neither CPU time nor disk IO
    // counters are requested, a single snapshot suffices and no sampling wait is needed.
    const bool sampleOnce = (options->showTypes & (FF_TOP_TYPE_CPU | FF_TOP_TYPE_DISK)) == 0;

    if (sampleOnce) {
        FF_LIST_AUTO_DESTROY snapshots = ffListCreate();
        const char* error = ffTopGetProcessSnapshot(&snapshots, options->showTypes);
        if (error) {
            FF_LIST_FOR_EACH (FFTopProcessSnapshot, item, snapshots) {
                ffStrbufDestroy(&item->name);
            }
            return error;
        }
        if (snapshots.length == 0) {
            return "No processes found";
        }

        FF_LIST_FOR_EACH (FFTopProcessSnapshot, snap, snapshots) {
            FFTopProcessResult* item = FF_LIST_ADD(FFTopProcessResult, *result);
            item->pid = snap->pid;
            item->memBytes = snap->memBytes;
            item->bytesRead = 0;
            item->bytesWritten = 0;
            item->cpuPercent = 0;
            item->startTime = snap->startTime;
            item->threads = snap->threads;
            ffStrbufInitMove(&item->name, &snap->name);
        }
    } else {
        if (startTick == 0) {
            ffPrepareTopProcesses(options->showTypes);
        }

        double elapsedTime = ffTimeGetTick() - startTick;
        if (elapsedTime < (double) options->waitTime) {
            ffTimeSleep(options->waitTime - (uint32_t) elapsedTime);
        }

        if (first.length == 0) {
            return "No processes found";
        }

        ffListReserve(result, sizeof(FFTopProcessResult), first.length < options->nProcesses ? first.length : options->nProcesses);

        FF_LIST_AUTO_DESTROY second = ffListCreateA(sizeof(FFTopProcessSnapshot), first.length);
        const char* error = ffTopGetProcessSnapshot(&second, options->showTypes);
        const double elapsed = ffTimeGetTick() - startTick;

        if (error || elapsed <= 0) {
            FF_LIST_FOR_EACH (FFTopProcessSnapshot, item, second) {
                ffStrbufDestroy(&item->name);
            }
            return error ?: "Invalid process sampling interval";
        }

        FF_LIST_FOR_EACH (FFTopProcessSnapshot, oldItem, first) {
            FFTopProcessSnapshot* newItem = nullptr;
            FF_LIST_FOR_EACH (FFTopProcessSnapshot, item, second) {
                if (item->pid == oldItem->pid) {
                    newItem = item;
                    break;
                }
            }
            if (!newItem || newItem->startTime != oldItem->startTime || newItem->cpuTime < oldItem->cpuTime || newItem->bytesRead < oldItem->bytesRead || newItem->bytesWritten < oldItem->bytesWritten) {
                ffStrbufDestroy(&oldItem->name);
                continue;
            }

            FFTopProcessResult* item = FF_LIST_ADD(FFTopProcessResult, *result);
            item->pid = newItem->pid;
            item->memBytes = newItem->memBytes;
            item->bytesRead = (newItem->bytesRead - oldItem->bytesRead) * 1000u / (uint64_t) elapsed;
            item->bytesWritten = (newItem->bytesWritten - oldItem->bytesWritten) * 1000u / (uint64_t) elapsed;
            item->cpuPercent = (double) (newItem->cpuTime - oldItem->cpuTime) / elapsed * 100.0;
            item->startTime = newItem->startTime;
            item->threads = newItem->threads;
            ffStrbufInitMove(&item->name, &oldItem->name);
        }

        // Reuse `second` as the baseline of the next call
        ffListDestroy(&first);
        ffListInitMove(&first, &second);
        startTick = ffTimeGetTick();
    }

    const void* compare = options->sort == FF_TOP_TYPE_DISK_WRITE ? (void*) compareDiskWriteResults
        : options->sort == FF_TOP_TYPE_DISK_READ                  ? (void*) compareDiskReadResults
        : options->sort == FF_TOP_TYPE_MEMORY                     ? (void*) compareMemoryResults
        : options->sort == FF_TOP_TYPE_START_TIME                 ? (void*) compareStartTimeResults
        : options->sort == FF_TOP_TYPE_THREADS                    ? (void*) compareThreadsResults
                                                                  : (void*) compareCpuResults;
    ffListSort(result, sizeof(FFTopProcessResult), (void*) compare);
    if (result->length > options->nProcesses) {
        FFTopProcessResult* items = (FFTopProcessResult*) result->data;
        for (uint32_t i = options->nProcesses; i < result->length; ++i) {
            ffStrbufDestroy(&items[i].name);
        }
        result->length = options->nProcesses;
    }

    return nullptr;
}
