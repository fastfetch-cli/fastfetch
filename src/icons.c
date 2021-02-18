#include "fastfetch.h"

void ffPrintIcons(FFstate* state)
{
    char plasma[256];
    ffParsePropFileHome(state, ".config/kdeglobals", "Theme=%[^\n]", plasma);
    
    char gtk2[256];
    ffParsePropFileHome(state, ".gtkrc-2.0", "gtk-icon-theme-name=\"%[^\"]+", gtk2);

    char gtk3[256];
    ffParsePropFileHome(state, ".config/gtk-3.0/settings.ini", "gtk-icon-theme-name=%[^\n]", gtk3);

    char gtk4[256];
    ffParsePropFileHome(state, ".config/gtk-4.0/settings.ini", "gtk-icon-theme-name=%[^\n]", gtk4);
    
    ffPrintLogoAndKey(state, "Icons");

    if(plasma[0] != '\0')
        printf("%s [Plasma], ", plasma);

    ffPrintGtkPretty(gtk2, gtk3, gtk4);
    putchar('\n');
}