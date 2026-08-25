#include "top.h"

#include "common/time.h"

static FFlist first;
static double startTick;
static FFTopTypes preparedShowTypes = FF_TOP_TYPE_CPU | FF_TOP_TYPE_MEMORY | FF_TOP_TYPE_DISK;

void ffPrepareTopProcesses(FFTopTypes showTypes) {
    if ((showTypes & (FF_TOP_TYPE_CPU | FF_TOP_TYPE_DISK)) == 0) {
        return; // Memory usage is instantaneous; no baseline snapshot is needed
    }

    if (startTick != 0 && preparedShowTypes == showTypes) {
        return; // Already prepared
    }

    if (startTick != 0) {
        // The set of requested types changed; discard the stale baseline
        FF_LIST_FOR_EACH (FFTopProcessSnapshot, item, first) {
            ffStrbufDestroy(&item->name);
        }
        ffListDestroy(&first);
    }

    ffListInit(&first);
    startTick = ffTimeGetTick();
    preparedShowTypes = showTypes;
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
// clang-format on

const char* ffDetectTopProcesses(FFTopOptions* options, FFlist* result) {
    ffListClear(result);
    if (options->nProcesses == 0) {
        return nullptr;
    }

    // Memory usage is instantaneous; when neither CPU time nor disk IO counters
    // are requested, a single snapshot suffices and no sampling wait is needed.
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
            ffStrbufInitMove(&item->name, &snap->name);
        }
    } else {
        if (startTick == 0 || preparedShowTypes != options->showTypes) {
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
