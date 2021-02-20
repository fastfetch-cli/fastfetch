#include "fastfetch.h"

void ffPrintHost(FFstate* state)
{
    FILE* nameFile = fopen("/sys/devices/virtual/dmi/id/product_name", "r");
    if(nameFile == NULL)
    {
        ffPrintError(state, "Host", "fopen(\"/sys/devices/virtual/dmi/id/product_name\", \"r\") == NULL");
        return;
    }
    char name[256];
    if(fscanf(nameFile, "%[^\n]", name) != 1)
    {
        ffPrintError(state, "Host", "fscanf(nameFile, \"%[^\\n]\", name) != 1");
        return;
    }
    fclose(nameFile);

    ffPrintLogoAndKey(state, "Host");
    printf(name);  

    FILE* versionFile = fopen("/sys/devices/virtual/dmi/id/product_version", "r");
    if(versionFile != NULL)
    {
        char version[256];
        if(fscanf(versionFile, "%[^\n]", version) == 1)
            printf(" %s", version);
        fclose(versionFile);
    }
    
    putchar('\n');
}