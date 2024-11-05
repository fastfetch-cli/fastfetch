extern "C"
{
#include "bluetooth.h"
}
#include "util/windows/wmi.hpp"
#include "util/windows/unicode.hpp"

#include <propvarutil.h>

STDAPI InitVariantFromStringArray(_In_reads_(cElems) PCWSTR *prgsz, _In_ ULONG cElems, _Out_ VARIANT *pvar);

static const char* detectWithWmi(FFlist* result)
{
    FFWmiQuery query(L"SELECT __PATH FROM Win32_PnPEntity WHERE Service = 'BthHFEnum'", nullptr, FFWmiNamespace::CIMV2);
    if(!query)
        return "Query WMI service failed";

    while(FFWmiRecord record = query.next())
    {
        IWbemClassObject* pInParams = nullptr;
        PCWSTR props[] = { L"{104EA319-6EE2-4701-BD47-8DDBF425BBE5} 2" };
        VARIANT devicePropertyKeys;
        InitVariantFromStringArray(props, ARRAY_SIZE(props), &devicePropertyKeys);
        record.obj->GetMethod(bstr_t(L"GetDeviceProperties"), 0, &pInParams, NULL);
        pInParams->Put(L"devicePropertyKeys", 0, &devicePropertyKeys, CIM_FLAG_ARRAY | CIM_STRING);
        IWbemCallResult* pResult = nullptr;
        query.pService->ExecMethod(bstr_t(record.get(L"__PATH").bstrVal), bstr_t(L"GetDeviceProperties"), 0, nullptr, pInParams, nullptr, &pResult);
        // TODO: parse result
    }
    return NULL;
}
