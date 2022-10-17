extern "C" {
#include "users.h"
}
#include "util/windows/wmi.hpp"

void ffDetectUsers(FFlist* users, FFstrbuf* error)
{
    FFWmiQuery query(L"SELECT Antecedent FROM Win32_LoggedOnUser", error);
    if(!query)
        return;

next:
    while(FFWmiRecord record = query.next())
    {
        FFstrbuf antecedent;
        ffStrbufInit(&antecedent);
        record.getString(L"Antecedent", &antecedent); // \\.\root\cimv2:Win32_Account.Domain="DOMAIN",Name="NAME"
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
}
