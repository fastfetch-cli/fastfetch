extern "C" {
#include "users.h"
}
#include "util/windows/wmi.hpp"

void ffDetectUsers(FFlist* users, FFstrbuf* error)
{
    IEnumWbemClassObject* pEnumerator = ffQueryWmi(L"SELECT Antecedent FROM Win32_LoggedOnUser", error);

    if(!pEnumerator)
        return;

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

next:
    while(SUCCEEDED(pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn)) && uReturn != 0)
    {
        FFstrbuf antecedent;
        ffStrbufInit(&antecedent);
        ffGetWmiObjString(pclsObj, L"Antecedent", &antecedent); // \\.\root\cimv2:Win32_Account.Domain="DOMAIN",Name="NAME"
        ffStrbufTrimRight(&antecedent, '"'); // \\.\root\cimv2:Win32_Account.Domain="DOMAIN",Name="NAME
        ffStrbufSubstrAfterFirstC(&antecedent, '"'); // DOMAIN",Name="NAME
        uint32_t index = ffStrbufFirstIndexC(&antecedent, '"');
        ffStrbufRemoveSubstr(&antecedent, index, ffStrbufLastIndexC(&antecedent, '"')); // DOMAIN"NAME
        antecedent.chars[index] = '\\';

        for(uint32_t i = 0; i < users->length; ++i)
        {
            if(ffStrbufComp((FFstrbuf*)ffListGet(users, i), &antecedent) == 0)
                goto next;
        }

        *(FFstrbuf*)ffListAdd(users) = antecedent;
    }

    if(users->length == 0)
        ffStrbufAppendS(error, "Unable to detect users");

    pclsObj->Release();
    pEnumerator->Release();
}
