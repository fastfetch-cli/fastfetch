#include "fastfetch.h"
#include "common/parsing.h"

bool ffStrSet(const char* str)
{
    if(str == NULL)
        return false;

    while(*str != '\0')
    {
        if(*str != ' ' && *str != '\t' && *str != '\n' && *str != '\r')
            return true;
    }

    return false;
}

void ffParseSemver(FFstrbuf* buffer, const FFstrbuf* major, const FFstrbuf* minor, const FFstrbuf* patch)
{
    if(major->length > 0)
        ffStrbufAppend(buffer, major);
    else if(minor->length > 0 || patch->length > 0)
        ffStrbufAppendC(buffer, '1');

    if(minor->length == 0 && patch->length == 0)
        return;

    ffStrbufAppendC(buffer, '.');

    if(minor->length > 0)
        ffStrbufAppend(buffer, minor);
    else if(patch->length > 0)
        ffStrbufAppendC(buffer, '0');

    if(patch->length == 0)
        return;

    ffStrbufAppendC(buffer, '.');

    ffStrbufAppend(buffer, patch);
}

int8_t ffVersionCompare(const FFVersion* version1, const FFVersion* version2)
{
    if(version1->major != version2->major)
        return version1->major > version2->major ? 1 : -1;

    if(version1->minor != version2->minor)
        return version1->minor > version2->minor ? 1 : -1;

    if(version1->patch != version2->patch)
        return version1->patch > version2->patch ? 1 : -1;

    return 0;
}

void ffVersionToPretty(const FFVersion* version, FFstrbuf* pretty)
{
    if(version->major > 0 || version->minor > 0 || version->patch > 0)
        ffStrbufAppendF(pretty, "%u", version->major);

    if(version->minor > 0 || version->patch > 0)
        ffStrbufAppendF(pretty, ".%u", version->minor);

    if(version->patch > 0)
        ffStrbufAppendF(pretty, ".%u", version->patch);
}

void ffParseGTK(FFstrbuf* buffer, const FFstrbuf* gtk2, const FFstrbuf* gtk3, const FFstrbuf* gtk4)
{
    if(gtk2->length > 0 && gtk3->length > 0 && gtk4->length > 0)
    {
        if((ffStrbufIgnCaseComp(gtk2, gtk3) == 0) && (ffStrbufIgnCaseComp(gtk2, gtk4) == 0))
        {
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK2/3/4]");
        }
        else if(ffStrbufIgnCaseComp(gtk2, gtk3) == 0)
        {
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK2/3], ");
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK4]");
        }
        else if(ffStrbufIgnCaseComp(gtk3, gtk4) == 0)
        {
            ffStrbufAppend(buffer, gtk2);
            ffStrbufAppendS(buffer, " [GTK2], ");
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK3/4]");
        }
        else
        {
            ffStrbufAppend(buffer, gtk2);
            ffStrbufAppendS(buffer, " [GTK2], ");
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK3], ");
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK4]");
        }
    }
    else if(gtk2->length > 0 && gtk3->length > 0)
    {
        if(ffStrbufIgnCaseComp(gtk2, gtk3) == 0)
        {
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK2/3]");
        }
        else
        {
            ffStrbufAppend(buffer, gtk2);
            ffStrbufAppendS(buffer, " [GTK2], ");
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK3]");
        }
    }
    else if(gtk3->length > 0 && gtk4->length > 0)
    {
        if(ffStrbufIgnCaseComp(gtk3, gtk4) == 0)
        {
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK3/4]");
        }
        else
        {
            ffStrbufAppend(buffer, gtk3);
            ffStrbufAppendS(buffer, " [GTK3], ");
            ffStrbufAppend(buffer, gtk4);
            ffStrbufAppendS(buffer, " [GTK4]");
        }
    }
    else if(gtk2->length > 0)
    {
        ffStrbufAppend(buffer, gtk2);
        ffStrbufAppendS(buffer, " [GTK2]");
    }
    else if(gtk3->length > 0)
    {
        ffStrbufAppend(buffer, gtk3);
        ffStrbufAppendS(buffer, " [GTK3]");
    }
    else if(gtk4->length > 0)
    {
        ffStrbufAppend(buffer, gtk4);
        ffStrbufAppendS(buffer, " [GTK4]");
    }
}
