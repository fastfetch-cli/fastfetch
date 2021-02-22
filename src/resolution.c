#include "fastfetch.h"

#include <X11/Xlib.h>

void ffPrintResolution(FFstate* state)
{
    if(ffPrintCachedValue(state, "Resolution"))
        return;

    Display* display = XOpenDisplay(NULL);
    Screen*   screen = DefaultScreenOfDisplay(display);

    char resolution[1024];
    sprintf(resolution, "%ix%i", screen->width, screen->height);

    ffSaveCachedValue(state, "Resolution", resolution);

    ffPrintLogoAndKey(state, "Resolution");
    puts(resolution);
}