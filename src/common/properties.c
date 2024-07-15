#include "fastfetch.h"
#include "common/properties.h"
#include "common/io/io.h"
#include "util/mallocHelper.h"

#include <stdlib.h>
#include <ctype.h>
#ifdef _WIN32
    #include "util/windows/getline.h"
#endif

bool ffParsePropLinePointer(const char** line, const char* start, FFstrbuf* buffer)
{
    if(**line == '\0')
        return false;

    //Skip any amount of whitespace at the begin of line
    while(**line == ' ' || **line == '\t')
        ++(*line);

    while(*start != '\0')
    {
        // Any amount of whitespace in the format string matches any amount of whitespace in the line, even none
        if(*start == ' ' || *start == '\t')
        {
            while(*start == ' ' || *start == '\t')
                ++start;

            while(**line == ' ' || **line == '\t')
                ++(*line);

            continue;
        }

        //Line doesn't match start, skip it
        if(tolower(**line) != tolower(*start) || **line == '\0')
            return false;

        //Line and start match, continue testing
        ++(*line);
        ++start;
    }

    char valueEnd = '\n';

    //Allow faster parsing of XML
    if(*(*line - 1) == '>')
        valueEnd = '<';

    //Skip any amount of whitespace at the begin of the value
    while(**line == ' ' || **line == '\t')
        ++(*line);

    //Allow faster parsing of quotet values
    if(**line == '"' || **line == '\'')
    {
        valueEnd = **line;
        ++(*line);
    }

    //Copy the value to the buffer
    while(**line != valueEnd && **line != '\n' && **line != '\0')
    {
        ffStrbufAppendC(buffer, **line);
        ++(*line);
    }

    ffStrbufTrimRight(buffer, ' ');

    return true;
}

bool ffParsePropLines(const char* lines, const char* start, FFstrbuf* buffer)
{
    while(!ffParsePropLinePointer(&lines, start, buffer))
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

// The following functions return true if the file was found, independently if start was found
// Buffers which already contain content are not overwritten
// The last occurrence of start in the first file will be the one used

bool ffParsePropFileValues(const char* filename, uint32_t numQueries, FFpropquery* queries)
{
    FF_AUTO_CLOSE_FILE FILE* file = fopen(filename, "r");
    if (file == NULL)
        return false;

    bool valueStorage[32];
    bool* unsetValues = valueStorage;

    if (numQueries > sizeof(valueStorage) / sizeof(valueStorage[0]))
        unsetValues = malloc(sizeof(bool) * numQueries);

    bool allSet = true;
    for (uint32_t i = 0; i < numQueries; i++)
    {
        unsetValues[i] = queries[i].buffer->length == 0;
        if (unsetValues[i])
            allSet = false;
    }

    if (!allSet)
    {
        FF_AUTO_FREE char* line = NULL;
        size_t len = 0;

        while (getline(&line, &len, file) != -1)
        {
            for(uint32_t i = 0; i < numQueries; i++)
            {
                if(!unsetValues[i])
                    continue;

                uint32_t currentLength = queries[i].buffer->length;
                queries[i].buffer->length = 0;
                if(!ffParsePropLine(line, queries[i].start, queries[i].buffer))
                    queries[i].buffer->length = currentLength;
            }
        }
    }

    if(unsetValues != valueStorage)
        free(unsetValues);
    return true;
}

bool ffParsePropFileHomeValues(const char* relativeFile, uint32_t numQueries, FFpropquery* queries)
{
    FF_STRBUF_AUTO_DESTROY absolutePath = ffStrbufCreateF("%s/%s", instance.state.platform.homeDir.chars, relativeFile);
    return ffParsePropFileValues(absolutePath.chars, numQueries, queries);
}

bool ffParsePropFileListValues(const FFlist* list, const char* relativeFile, uint32_t numQueries, FFpropquery* queries)
{
    bool foundAFile = false;

    FF_LIST_FOR_EACH(FFstrbuf, dirPrefix, *list)
    {
        const uint32_t dirPrefixLength = dirPrefix->length;
        ffStrbufAppendS(dirPrefix, relativeFile);
        if(ffParsePropFileValues(dirPrefix->chars, numQueries, queries))
            foundAFile = true;
        ffStrbufSubstrBefore(dirPrefix, dirPrefixLength);

        bool allSet = true;
        for(uint32_t k = 0; k < numQueries; k++)
        {
            if(queries[k].buffer->length == 0)
            {
                allSet = false;
                break;
            }
        }

        if(allSet)
            break;
    }

    return foundAFile;
}
