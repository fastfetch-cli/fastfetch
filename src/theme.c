#include "fastfetch.h"

#include <string.h>

void ffPrintTheme(FFstate* state)
{
    char plasma[256];
    ffParsePropFileHome(state, ".config/kdeglobals", "Name=%[^\n]", plasma);
    
    char gtk2[256];
    ffParsePropFileHome(state, ".gtkrc-2.0", "gtk-theme-name=\"%[^\"]+", gtk2);
    if(gtk2[0] == '\0')
        strcpy(gtk2, "Raleigh");

    char gtk3[256];
    ffParsePropFileHome(state, ".config/gtk-3.0/settings.ini", "gtk-theme-name=%[^\n]", gtk3);
    if(gtk3[0] == '\0')
        strcpy(gtk3, "Adwaita");

    char gtk4[256];
    ffParsePropFileHome(state, ".config/gtk-4.0/settings.ini", "gtk-theme-name=%[^\n]", gtk4);
    if(gtk4[0] == '\0')
        strcpy(gtk4, "Adwaita");
    
    ffPrintLogoAndKey(state, "Theme");

    if(plasma[0] == '\0')
        printf("Breeze [Plasma], ");
    else
        printf("%s [Plasma], ", plasma);

    ffPrintGtkPretty(gtk2, gtk3, gtk4);
    putchar('\n');
}