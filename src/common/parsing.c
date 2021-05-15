#include "fastfetch.h"

void ffGetGtkPretty(FFstrbuf* buffer, const FFstrbuf* gtk2, const FFstrbuf* gtk3, const FFstrbuf* gtk4)
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

void ffGetFont(const char* font, FFstrbuf* name, double* size)
{
    ffStrbufEnsureCapacity(name, 32);

    int scanned = sscanf(font, "%31[^,], %lf", name->chars, size);

    ffStrbufRecalculateLength(name);

    if(scanned < 2)
        *size = 0.0;

    if(scanned == 0)
        ffStrbufSetS(name, font);
}

void ffGetFontPretty(FFstrbuf* buffer, const FFstrbuf* name, double size)
{
    ffStrbufAppend(buffer, name);

    if(size > 0)
    {
        ffStrbufAppendS(buffer, " (");
        ffStrbufAppendF(buffer, "%g", size);
        ffStrbufAppendS(buffer, "pt)");
    }
}

bool ffGetPropValue(const char* line, const char* start, FFstrbuf* buffer)
{
    uint32_t lineIndex = 0;
    uint32_t startIndex = 0;

    //Skip any amount of whitespace at the begin of line
    while(line[lineIndex] == ' ' || line[lineIndex] == '\t')
        ++lineIndex;

    while(start[startIndex] != '\0')
    {
        // Any amount of whitespace in the format string matches any amount of whitespace in the line
        if(start[startIndex] == ' ' || start[startIndex] == '\t')
        {
            while(start[startIndex] == ' ' || start[startIndex] == '\t')
                ++startIndex;

            while(line[lineIndex] == ' ' || line[lineIndex] == '\t')
                ++lineIndex;

            continue;
        }

        if(line[lineIndex] == '\0' || line[lineIndex] != start[startIndex])
            return false;

        ++lineIndex;
        ++startIndex;
    }

    //Allow quotet values
    const char* quotes = NULL;
    if(line[lineIndex] == '"' || line[lineIndex] == '\'')
        quotes = &line[lineIndex++];

    //Copy the value to the buffer
    while(line[lineIndex] != '\n' && line[lineIndex] != '\0' && (quotes == NULL || *quotes != line[lineIndex]))
        ffStrbufAppendC(buffer, line[lineIndex++]);

    ffStrbufTrim(buffer, ' ');

    return true;
}
