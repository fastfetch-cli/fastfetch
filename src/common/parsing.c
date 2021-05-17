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

static bool getPropValueLine(const char** linePtr, const char* start, FFstrbuf* buffer)
{
    const char* line = *linePtr;

    if(*line == '\0')
        return false;

    //Skip any amount of whitespace at the begin of line
    while(*line == ' ' || *line == '\t')
        ++line;

    while(*start != '\0')
    {
        // Any amount of whitespace in the format string matches any amount of whitespace in the line, even none
        if(*start == ' ' || *start == '\t')
        {
            while(*start == ' ' || *start == '\t')
                ++start;

            while(*line == ' ' || *line == '\t')
                ++line;

            continue;
        }

        //Line doesn't match start, skip it
        if(*line != *start || *line == '\0')
            return false;

        //Line and start match, continue testing
        ++line;
        ++start;
    }

    char valueEnd = '\n';

    //Allow faster parsing of XML
    if(*(line - 1) == '>')
        valueEnd = '<';

    //Skip any amount of whitespace at the begin of the value
    while(*line == ' ' || *line == '\t')
        ++line;

    //Allow faster parsing of quotet values
    if(*line == '"' || *line == '\'')
    {
        valueEnd = *line;
        ++line;
    }

    //Copy the value to the buffer
    while(*line != valueEnd && *line != '\n' && *line != '\0')
    {
        ffStrbufAppendC(buffer, *line);
        ++line;
    }

    ffStrbufTrimRight(buffer, ' ');

    return true;
}

bool ffGetPropValue(const char* line, const char* start, FFstrbuf* buffer)
{
    return getPropValueLine(&line, start, buffer);
}

bool ffGetPropValueFromLines(const char* lines, const char* start, FFstrbuf* buffer)
{
    while(!getPropValueLine(&lines, start, buffer))
    {
        while(*lines != '\0' && *lines != '\n')
            ++lines;

        if(*lines == '\0')
            return false;

        //Skip '\n'
        ++lines;
    }

    return true;
}
