#include "displayserver.h"
#include "detection/internal.h"

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds, const FFinstance* instance);

const FFDisplayServerResult* ffConnectDisplayServer(const FFinstance* instance)
{
    FF_DETECTION_INTERNAL_GUARD(FFDisplayServerResult,
        ffConnectDisplayServerImpl(&result, instance);
    );
}
