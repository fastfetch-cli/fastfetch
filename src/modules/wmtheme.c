#include "fastfetch.h"
#include "string.h"
#include "libxml/parser.h"

#define FF_WMTHEME_MODULE_NAME "WM Theme"
#define FF_WMTHEME_NUM_FORMAT_ARGS 1

static void printWMTheme(FFinstance* instance, const char* theme)
{
    if(instance->config.wmThemeFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmThemeKey);
        puts(theme);
    }
    else
    {
        ffPrintFormatString(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmThemeKey, &instance->config.wmThemeFormat, NULL, FF_WMTHEME_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRING, theme}
        });
    }
}

static void printKWin(FFinstance* instance)
{
    char theme[256];
    theme[0] = '\0';
    ffParsePropFileHome(instance, ".config/kwinrc", "theme=%s", theme);

    if(theme[0] == '\0')
    {
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmThemeKey, &instance->config.wmThemeFormat, FF_WMTHEME_NUM_FORMAT_ARGS, "Couldn't find \"theme=\" in \".config/kwinrc\"");
        return;
    }

    printWMTheme(instance, theme);
}

xmlNode* ffXmlFindNode(xmlNode* node, const xmlChar* name)
{
    while (node) {
        if (!xmlStrcmp(node->name, (const xmlChar*) name))
            return node;
        node = node->next;
    }
    return NULL;
}

static void printOpenbox(FFinstance* instance)
{
	xmlDoc* document;
	xmlNode* node;
	char fullPath[64];
	char theme[256];
	theme[0] = '\0';

	strcpy(fullPath, instance->state.passwd->pw_dir);
	// Is there a better way to get deName internally?
	const char* deName = getenv("XDG_SESSION_DESKTOP");

	if (strcmp(deName, (const char*)"LXQt Desktop") == 0)
		strcat(fullPath, "/.config/openbox/lxqt-rc.xml");
	else if(strcmp(deName, (const char*)"LXDE") == 0)
		strcat(fullPath, "/.config/openbox/lxde-rc.xml");
	else 
		strcat(fullPath, "/.config/openbox/rc.xml");

	document = xmlReadFile(fullPath, NULL, XML_PARSE_SAX1 | XML_PARSE_NOBLANKS | XML_PARSE_RECOVER);
	node = xmlDocGetRootElement(document)->children;

	if ((node = ffXmlFindNode(node, (const xmlChar*)"theme")) != NULL)
	{
		if ((node = ffXmlFindNode(node->xmlChildrenNode, (const xmlChar*)"name")) != NULL)
			strcpy(theme, (char*)xmlNodeGetContent(node));
	}
	xmlFreeDoc(document);

	if(theme[0] == '\0')
	{
		ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmThemeKey, &instance->config.wmThemeFormat, FF_WMTHEME_NUM_FORMAT_ARGS, "Couldn't find theme name in \"%s\"", fullPath);
		return;
	}

	printWMTheme(instance, theme);
}

void ffPrintWMTheme(FFinstance* instance)
{
    const FFWMResult* result = ffCalculateWM(instance);

    if(result->prettyName.length == 0)
    {
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmThemeKey, &instance->config.wmThemeFormat, FF_WMTHEME_NUM_FORMAT_ARGS, "WM Theme needs sucessfull WM detection");
        return;
    }

    if(ffStrbufIgnCaseCompS(&result->prettyName, "KWin") == 0)
        printKWin(instance);
    else if(ffStrbufIgnCaseCompS(&result->prettyName, "Openbox") == 0)
        printOpenbox(instance);
    else
        ffPrintError(instance, FF_WMTHEME_MODULE_NAME, 0, &instance->config.wmThemeKey, &instance->config.wmThemeFormat, FF_WMTHEME_NUM_FORMAT_ARGS, "Unknown WM: %s", result->prettyName.chars);
}
