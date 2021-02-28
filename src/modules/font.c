#include "fastfetch.h"

static void parseFont(char* font, char* buffer)
{
    char name[64];
    char size[32];
    int scanned = sscanf(font, "%[^,], %[^,]", name, size);
    
    if(scanned == 0)
        strcpy(buffer, font);
    else if(scanned == 1)
        strcpy(buffer, name);
    else
        sprintf(buffer, "%s (%spt)", name, size);
}

void ffPrintFont(FFinstance* instance)
{
    #ifdef FASTFETCH_BUILD_FLASHFETCH
    if(ffPrintCustomValue(instance, "Font"))
        return;
    #endif // FASTFETCH_BUILD_FLASHFETCH

    char plasma[256];
    ffParsePropFileHome(instance, ".config/kdeglobals", "font=%[^\n]", plasma);

    char gtk2[256];
    ffParsePropFileHome(instance, ".gtkrc-2.0", "gtk-font-name=\"%[^\"]+", gtk2);
    
    char gtk2Pretty[256];
    if(gtk2[0] == '\0')
        gtk2Pretty[0] = '\0';
    else
        parseFont(gtk2, gtk2Pretty);

    char gtk3[256];
    ffParsePropFileHome(instance, ".config/gtk-3.0/settings.ini", "gtk-font-name=%[^\n]", gtk3);
    
    char gtk3Pretty[256];
    if(gtk3[0] == '\0')
        gtk3Pretty[0] = '\0';
    else
        parseFont(gtk3, gtk3Pretty);

    char gtk4[256];
    ffParsePropFileHome(instance, ".config/gtk-4.0/settings.ini", "gtk-font-name=%[^\n]", gtk4);

    char gtk4Pretty[256];
    if(gtk4Pretty[0] == '\0')
        gtk4Pretty[0] = '\0';
    else
        parseFont(gtk4, gtk4Pretty);
    
    ffPrintLogoAndKey(instance, "Font");

    if(plasma[0] == '\0')
    {
        printf("Noto Sans (10pt) [Plasma], ");
    }
    else
    {
        char plasmaPretty[256];
        parseFont(plasma, plasmaPretty);
        printf("%s [Plasma], ", plasmaPretty);
    }

    ffPrintGtkPretty(gtk2Pretty, gtk3Pretty, gtk4Pretty);
    putchar('\n');
}