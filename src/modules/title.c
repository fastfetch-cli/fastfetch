#include "fastfetch.h"

#include <string.h>

void ffPrintTitle(FFstate* state)
{
    char hostname[256];
    gethostname(hostname, 256);

    state->titleLength = strlen(state->passwd->pw_name) + 1 + strlen(hostname);

    ffPrintLogoLine(state);

    printf(FASTFETCH_TEXT_MODIFIER_BOLT"%s%s"FASTFETCH_TEXT_MODIFIER_RESET"@"FASTFETCH_TEXT_MODIFIER_BOLT"%s%s"FASTFETCH_TEXT_MODIFIER_RESET"\n",
        state->color, state->passwd->pw_name, state->color, hostname
    );
}