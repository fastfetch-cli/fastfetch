#include "os.h"
#include "common/io/io.h"

#include <OS.h>

void ffDetectOSImpl(FFOSResult* os)
{
    system_info info;

    ffStrbufSetStatic(&os->name, "Haiku");
    ffStrbufSetStatic(&os->prettyName, "Haiku");

    ffStrbufSetStatic(&os->id, "haiku");

    if (get_system_info(&info) != B_OK)
        return;
    ffStrbufAppendF(&os->version, "R%ld", info.kernel_version);

    // TODO: check kernel resources?
}
