#include "security.h"

const char* ffDetectSecurity(FFSecurityResult* result)
{
    ffStrbufInit(&result->tpmStatus);
    ffStrbufInit(&result->secureBootStatus);
    ffStrbufSetStatic(&result->tpmStatus, "TPM Not Available");
    ffStrbufSetStatic(&result->secureBootStatus, "Secure Boot Unknown");
    return NULL;
}
