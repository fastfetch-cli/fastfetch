#include "common/kmod.h"
#include "common/stringUtils.h"

#include <sys/module.h>
#include <sys/param.h>

typedef struct __attribute__((__packed__)) FFNbsdModList {
    int len;
    modstat_t mods[];
} FFNbsdModList;

bool ffKmodLoaded(const char* modName) {
    static FFNbsdModList* list = NULL;

    if (list == NULL) {
        struct iovec iov = {};

        for (size_t len = 8192;; len = iov.iov_len) {
            iov.iov_len = len;
            iov.iov_base = realloc(iov.iov_base, len);
            if (modctl(MODCTL_STAT, &iov) < 0) {
                free(iov.iov_base);
                return true; // ignore errors
            }

            if (len >= iov.iov_len) {
                break;
            }
        }
        list = (FFNbsdModList*) iov.iov_base;
    }

    for (int i = 0; i < list->len; i++) {
        if (ffStrEquals(list->mods[i].ms_name, modName)) {
            return true;
        }
    }

    return false;
}
