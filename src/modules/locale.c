#include "fastfetch.h"

void ffPrintLocale(FFinstance* instance)
{
	if(ffPrintCachedValue(instance, "Locale"))
		return;

	char localeCode[256];

	/* Try to open /etc/locale.conf in read-only mode */
	FILE *fp;
	fp = fopen("/etc/locale.conf", "r");

	/* File does not exist */
	if (fp == NULL) {
		/* Check if LANG exists */
		if (getenv("LANG") != NULL)
			strcpy(localeCode, getenv("LANG"));
		/* Check if LC_ALL exists */
		else if (getenv("LC_ALL") != NULL)
			strcpy(localeCode, getenv("LC_ALL"));
		/* Check if LC_CTYPE exists */
		else if (getenv("LC_CTYPE") != NULL)
			strcpy(localeCode, getenv("LC_CTYPE"));
		/* Check if LC_CTYPE exists */
		else if (getenv("LC_CTYPE") != NULL)
			strcpy(localeCode, getenv("LC_CTYPE"));
		/* Check if LC_MESSAGES exists */
		else if (getenv("LC_MESSAGES") != NULL)
			strcpy(localeCode, getenv("LC_MESSAGES"));
		/* User has broken system */
		else
			ffPrintError(instance, "Locale", "Locale not found!");
		/* /etc/locale.conf exists */
	} else {
		ffParsePropFile("/etc/locale.conf", "LANG=%[^\n]", localeCode);

		/* Free pointer to file */
		fclose(fp);
	}

	FF_STRBUF_CREATE(locale);

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


	ffPrintAndSaveCachedValue(instance, "Locale", &locale);
	ffStrbufDestroy(&locale);
}
