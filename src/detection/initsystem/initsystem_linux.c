#include "initsystem.h"
#include "common/processing.h"

const char* ffDetectInitSystem(FFInitSystemResult* result)
{
    const char* error = ffProcessGetBasicInfoLinux((int) result->pid, &result->name, NULL, NULL);
    if (error) return error;

    const char* exeName;

    ffProcessGetInfoLinux((int) result->pid, &result->name, &result->exe, &exeName, NULL);

    return NULL;
}
