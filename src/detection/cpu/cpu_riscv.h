#pragma once

#include "common/properties.h"

// Unlike frequency-based core grouping, uarch also distinguishes different
// implementations running at the same frequency. Keep the kernel's names:
// a microarchitecture name alone does not establish a P/E classification.
static bool ffCPUDetectRiscvUarch(FFstrbuf* cpuinfo, uint16_t coresOnline, FFstrbuf* name) {
    typedef struct UarchCount {
        FFstrbuf name;
        uint32_t count;
    } UarchCount;

    FF_LIST_AUTO_DESTROY groups = ffListCreate();
    FF_STRBUF_AUTO_DESTROY value = ffStrbufCreate();
    uint32_t processors = 0;
    uint32_t descriptions = 0;
    bool haveUarch = false;
    bool valid = true;
    char* line = NULL;
    size_t length = 0;

    while (ffStrbufGetline(&line, &length, cpuinfo)) {
        ffStrbufClear(&value);
        if (ffParsePropLine(line, "processor :", &value)) {
            if (processors > 0 && !haveUarch) {
                valid = false;
            }
            ++processors;
            haveUarch = false;
        } else if (ffParsePropLine(line, "uarch :", &value)) {
            if (processors == 0 || haveUarch || value.length == 0) {
                valid = false;
                continue;
            }
            haveUarch = true;
            ++descriptions;

            UarchCount* group = NULL;
            FF_LIST_FOR_EACH (UarchCount, candidate, groups) {
                if (ffStrbufEqual(&candidate->name, &value)) {
                    group = candidate;
                    break;
                }
            }
            if (!group) {
                group = FF_LIST_ADD(UarchCount, groups);
                ffStrbufInitCopy(&group->name, &value);
                group->count = 0;
            }
            ++group->count;
        }
    }

    // /proc/cpuinfo describes online CPUs. Do not publish a partial breakdown
    // if descriptions are missing or CPU hotplug changed the snapshot.
    bool heterogeneous = valid && haveUarch && groups.length > 1 &&
        processors == coresOnline && descriptions == coresOnline;
    if (heterogeneous) {
        bool haveSocName = name->length > 0;
        if (haveSocName) {
            ffStrbufAppendS(name, " (");
        }
        FF_LIST_FOR_EACH (UarchCount, group, groups) {
            if (group != ffListGet(&groups, sizeof(UarchCount), 0)) {
                ffStrbufAppendS(name, " + ");
            }
            ffStrbufAppendF(name, "%u x %s", group->count, group->name.chars);
        }
        if (haveSocName) {
            ffStrbufAppendC(name, ')');
        }
    }

    FF_LIST_FOR_EACH (UarchCount, group, groups) {
        ffStrbufDestroy(&group->name);
    }
    return heterogeneous;
}
