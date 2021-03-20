#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fastfetch.h"

void ffPrintLocale(FFinstance* instance)
{
    if(ffPrintCachedValue(instance, "Locale"))
        return;

   char localeCode[256];

    FILE *fp;
    /* Try to open /etc/locale.conf in read-only mode */
    fp = fopen("/etc/locale.conf", "r");

    /* File does not exist */
    if (fp == NULL) {
	    strcpy(localeCode, getenv("LANG"));
    } else {
    	ffParsePropFile("/etc/locale.conf", "LANG=%[^\n]", localeCode);
    }

    /* Free pointer to file */
    fclose(fp);

    ffPrintLogoAndKey(instance, "Locale");

    FFstrbuf locale;
    ffStrbufInit(&locale);

    if(ffStrbufIsEmpty(&instance->config.localeFormat))
    {
        ffStrbufSetS(&locale, localeCode);
    }
    else
    {
        ffParseFormatString(&locale, &instance->config.localeFormat, 1,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, localeCode}
        );
    }


    ffPrintAndSaveCachedValue(instance, "Locale", locale.chars);
    ffStrbufDestroy(&locale);
}
