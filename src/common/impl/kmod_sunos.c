#include "common/kmod.h"
#include "common/stringUtils.h"

#include <sys/modctl.h>
#include <errno.h>

bool ffKmodLoaded(const char* modName) {
    struct modinfo modinfo = {
        .mi_id = -1,
        .mi_nextid = -1,
        .mi_info = MI_INFO_ALL,
    };

    for (int id = -1; modctl(MODINFO, id, &modinfo) == 0; id = modinfo.mi_id) {
        modinfo.mi_name[MODMAXNAMELEN - 1] = '\0';

        if (ffStrEquals(modinfo.mi_name, modName)) {
            return true;
        }
    }

    return !(errno == EINVAL || errno == ENOENT);
}
