#include "fastfetch.h"

#include <X11/Xlib.h>

void ffPrintResolution(FFstate* state)
{

    Display* display = XOpenDisplay(NULL);
    Screen*   screen = DefaultScreenOfDisplay(display);

    ffPrintLogoAndKey(state, "Resolution");
    printf("%ix%i\n", screen->width, screen->height);
}