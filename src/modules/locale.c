#include "fastfetch.h"

#include <string.h>

#define FF_LOCALE_MODULE_NAME "Locale"
#define FF_LOCALE_NUM_FORMAT_ARGS 1

void ffPrintLocale(FFinstance* instance)
{
	if(ffPrintFromCache(instance, FF_LOCALE_MODULE_NAME, &instance->config.localeKey, &instance->config.localeFormat, FF_LOCALE_NUM_FORMAT_ARGS))
        return;

	char localeCode[256];

	FILE *fp;
	fp = fopen("/etc/locale.conf", "r");

	if (fp == NULL) {
		if (getenv("LANG") != NULL)
			strcpy(localeCode, getenv("LANG"));
		else if (getenv("LC_ALL") != NULL)
			strcpy(localeCode, getenv("LC_ALL"));
		else if (getenv("LC_CTYPE") != NULL)
			strcpy(localeCode, getenv("LC_CTYPE"));
		else if (getenv("LC_CTYPE") != NULL)
			strcpy(localeCode, getenv("LC_CTYPE"));
		else if (getenv("LC_MESSAGES") != NULL)
			strcpy(localeCode, getenv("LC_MESSAGES"));
	} else {
		ffParsePropFile("/etc/locale.conf", "LANG=%[^\n]", localeCode);
		fclose(fp);
	}

    if(localeCode[0] == '\0')
    {
        ffPrintError(instance, FF_LOCALE_MODULE_NAME, 0, &instance->config.localeKey, &instance->config.localeFormat, FF_LOCALE_NUM_FORMAT_ARGS, "No locale found");
        return;
    }

	FF_STRBUF_CREATE(locale);
    ffStrbufSetS(&locale, localeCode);

    ffPrintAndSaveToCache(instance, FF_LOCALE_MODULE_NAME, &instance->config.localeKey, &locale, &instance->config.localeFormat, FF_LOCALE_NUM_FORMAT_ARGS, (FFformatarg[]){
        {FF_FORMAT_ARG_TYPE_STRING, localeCode}
    });

	ffStrbufDestroy(&locale);
}
