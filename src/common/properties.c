#include "fastfetch.h"

static bool parsePropLinePointer(const char** line, const char* start, FFstrbuf* buffer)
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
        if(**line != *start || **line == '\0')
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

bool ffParsePropLine(const char* line, const char* start, FFstrbuf* buffer)
{
    return parsePropLinePointer(&line, start, buffer);
}

bool ffParsePropLines(const char* lines, const char* start, FFstrbuf* buffer)
{
    while(!parsePropLinePointer(&lines, start, buffer))
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
// The last occurence of start in the first file will be the one used
// The *Values methods always return true, if all properties were already found before, without testing if the file exists

bool ffParsePropFileValues(const char* filename, uint32_t numQueries, FFpropquery* queries)
{
    bool valueStorage[4];
    bool* unsetValues;

    if(numQueries > sizeof(valueStorage) / sizeof(valueStorage[0]))
        unsetValues = malloc(sizeof(bool) * numQueries);
    else
        unsetValues = valueStorage;

    bool allSet = true;
    for(uint32_t i = 0; i < numQueries; i++)
    {
        if((unsetValues[i] = queries[i].buffer->length == 0))
            allSet = false;
    }

    FILE* file = fopen(filename, "r");
    if(file == NULL)
        return false;

    if(allSet)
    {
        fclose(file);
        if(unsetValues != valueStorage)
            free(unsetValues);
        return true;
    }

    char* line = NULL;
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

    if(line != NULL)
        free(line);

    fclose(file);

    if(unsetValues != valueStorage)
        free(unsetValues);

    return true;
}

bool ffParsePropFile(const char* filename, const char* start, FFstrbuf* buffer)
{
    return ffParsePropFileValues(filename, 1, (FFpropquery[]){{start, buffer}});
}

bool ffParsePropFileHomeValues(const FFinstance* instance, const char* relativeFile, uint32_t numQueries, FFpropquery* queries)
{
    FFstrbuf absolutePath;
    ffStrbufInitA(&absolutePath, 64);
    ffStrbufAppendS(&absolutePath, instance->state.passwd->pw_dir);
    ffStrbufAppendC(&absolutePath, '/');
    ffStrbufAppendS(&absolutePath, relativeFile);

    bool result = ffParsePropFileValues(absolutePath.chars, numQueries, queries);

    ffStrbufDestroy(&absolutePath);

    return result;
}

bool ffParsePropFileHome(const FFinstance* instance, const char* relativeFile, const char* start, FFstrbuf* buffer)
{
    return ffParsePropFileHomeValues(instance, relativeFile, 1, (FFpropquery[]){{start, buffer}});
}

bool ffParsePropFileConfigValues(const FFinstance* instance, const char* relativeFile, uint32_t numQueries, FFpropquery* queries)
{
    bool foundAFile = false;

    for(uint32_t i = 0; i < instance->state.configDirs.length; i++)
    {
        FFstrbuf* baseDir = (FFstrbuf*) ffListGet(&instance->state.configDirs, i);
        uint32_t baseDirLength = baseDir->length;

        if(*relativeFile != '/')
            ffStrbufAppendC(baseDir, '/');

        ffStrbufAppendS(baseDir, relativeFile);

        if(ffParsePropFileValues(baseDir->chars, numQueries, queries))
            foundAFile = true;

        ffStrbufSubstrBefore(baseDir, baseDirLength);

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

bool ffParsePropFileConfig(const FFinstance* instance, const char* relativeFile, const char* start, FFstrbuf* buffer)
{
    return ffParsePropFileConfigValues(instance, relativeFile, 1, (FFpropquery[]){{start, buffer}});
}
