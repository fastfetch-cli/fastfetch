#include "fastfetch.h"

#include <string.h>

void ffPrintTitle(FFinstance* instance)
{
    char hostname[256];
    gethostname(hostname, 256);

    instance->config.titleLength = strlen(instance->state.passwd->pw_name) + 1 + strlen(hostname);

    ffPrintLogoLine(instance);

    printf(FASTFETCH_TEXT_MODIFIER_BOLT"%s%s"FASTFETCH_TEXT_MODIFIER_RESET"@"FASTFETCH_TEXT_MODIFIER_BOLT"%s%s"FASTFETCH_TEXT_MODIFIER_RESET"\n",
        instance->config.color, instance->state.passwd->pw_name, instance->config.color, hostname
    );
}