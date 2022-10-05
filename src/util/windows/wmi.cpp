#include "wmi.hpp"

#include <synchapi.h>

//https://learn.microsoft.com/en-us/windows/win32/wmisdk/example--getting-wmi-data-from-the-local-computer
//https://learn.microsoft.com/en-us/windows/win32/cimwin32prov/computer-system-hardware-classes
static void CoUninitializeWrap()
{
    CoUninitialize();
}

static BOOL CALLBACK InitHandleFunction(PINIT_ONCE, PVOID, PVOID *lpContext)
{
    static char error[128];
    *((char**)lpContext) = error;

    HRESULT hres;

    // Initialize COM
    hres = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    // Set general COM security levels
    hres = CoInitializeSecurity(
        nullptr,
        -1,                          // COM authentication
        nullptr,                     // Authentication services
        nullptr,                     // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
        nullptr,                     // Authentication info
        EOAC_NONE,                   // Additional capabilities
        nullptr                      // Reserved
        );

    if (FAILED(hres))
    {
        CoUninitialize();
        snprintf(error, sizeof(error), "Failed to initialize security. Error code = 0x%X", hres);
        return FALSE;
    }

    // Obtain the initial locator to WMI
    IWbemLocator* pLoc = nullptr;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*) &pLoc);

    if (FAILED(hres))
    {
        CoUninitialize();
        snprintf(error, sizeof(error), "Failed to create IWbemLocator object. Error code = 0x%X", hres);
        return FALSE;
    }

    // Connect to WMI through the IWbemLocator::ConnectServer method
    IWbemServices* pSvc = nullptr;

    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
         bstr_t(L"ROOT\\CIMV2"),  // Object path of WMI namespace
         nullptr,                 // User name. nullptr = current user
         nullptr,                 // User password. nullptr = current
         0,                       // Locale. nullptr indicates current
         0,                       // Security flags.
         0,                       // Authority (for example, Kerberos)
         0,                       // Context object
         &pSvc                    // pointer to IWbemServices proxy
         );
    pLoc->Release();
    pLoc = nullptr;

    if (FAILED(hres))
    {
        CoUninitialize();
        snprintf(error, sizeof(error), "Could not connect WMI server. Error code = 0x%X", hres);
        return FALSE;
    }

    // Set security levels on the proxy -------------------------
    hres = CoSetProxyBlanket(
       pSvc,                        // Indicates the proxy to set
       RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
       RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
       nullptr,                     // Server principal name
       RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
       RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
       nullptr,                     // client identity
       EOAC_NONE                    // proxy capabilities
    );

    if (FAILED(hres))
    {
        pSvc->Release();
        CoUninitialize();
        snprintf(error, sizeof(error), "Could not set proxy blanket. Error code = 0x%X", hres);
        return FALSE;
    }

    *((IWbemServices**)lpContext) = pSvc;
    atexit(CoUninitializeWrap);
    return TRUE;
}


IEnumWbemClassObject* ffQueryWmi(const wchar_t* queryStr, FFstrbuf* error)
{
    static INIT_ONCE s_InitOnce = INIT_ONCE_STATIC_INIT;
    const char* context;
    if (InitOnceExecuteOnce(&s_InitOnce, &InitHandleFunction, nullptr, (void**)&context) == FALSE)
    {
        if(error)
            ffStrbufInitS(error, context);
        return nullptr;
    }

    // Use the IWbemServices pointer to make requests of WMI
    IEnumWbemClassObject* pEnumerator = nullptr;
    HRESULT hres;

    hres = ((IWbemServices*)context)->ExecQuery(
        bstr_t(L"WQL"),
        bstr_t(queryStr),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &pEnumerator);

    if (FAILED(hres))
    {
        if(error)
            ffStrbufAppendF(error, "Query for '%ls' failed. Error code = 0x%X", queryStr, hres);
        return nullptr;
    }

    return pEnumerator;
}

void ffBstrToStrbuf(BSTR bstr, FFstrbuf* strbuf) {
    int len = (int)SysStringLen(bstr);
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, bstr, len, nullptr, 0, nullptr, nullptr);
    ffStrbufEnsureFree(strbuf, (uint32_t)size_needed);
    WideCharToMultiByte(CP_UTF8, 0, bstr, len, strbuf->chars, size_needed, nullptr, nullptr);
    strbuf->length = (uint32_t)size_needed;
}

bool ffGetWmiObjValue(IWbemClassObject* obj, const wchar_t* key, FFstrbuf* strbuf)
{
    bool result = true;

    VARIANT vtProp;
    VariantInit(&vtProp);

    CIMTYPE type;
    if(FAILED(obj->Get(key, 0, &vtProp, &type, nullptr)) || vtProp.vt == VT_EMPTY || vtProp.vt == VT_NULL)
    {
        result = false;
    }
    else
    {
        switch(type)
        {
            case CIM_ILLEGAL:
            case CIM_EMPTY: result = false; break;
            case CIM_SINT8: ffStrbufAppendF(strbuf, "%d", (int)vtProp.cVal); break;
            case CIM_SINT16: ffStrbufAppendF(strbuf, "%d", (int)vtProp.iVal); break;
            case CIM_SINT32: ffStrbufAppendF(strbuf, "%d", (int)vtProp.intVal); break;
            case CIM_SINT64: ffStrbufAppendF(strbuf, "%lld", vtProp.llVal); break;
            case CIM_UINT8: ffStrbufAppendF(strbuf, "%u", (unsigned)vtProp.bVal); break;
            case CIM_UINT16: ffStrbufAppendF(strbuf, "%u", (unsigned)vtProp.uiVal); break;
            case CIM_UINT32: ffStrbufAppendF(strbuf, "%u", (unsigned)vtProp.uintVal); break;
            case CIM_UINT64: ffStrbufAppendF(strbuf, "%llu", vtProp.ullVal); break;
            case CIM_REAL32: ffStrbufAppendF(strbuf, "%f", vtProp.fltVal); break;
            case CIM_REAL64: ffStrbufAppendF(strbuf, "%f", vtProp.dblVal); break;
            case CIM_BOOLEAN: ffStrbufAppendF(strbuf, "%s", vtProp.boolVal ? "True" : "False"); break;
            case CIM_STRING: ffBstrToStrbuf(vtProp.bstrVal, strbuf); break;
            case CIM_DATETIME: {
                ISWbemDateTime *pDateTime;
                BSTR dateStr;
                if(FAILED(CoCreateInstance(__uuidof(SWbemDateTime), 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDateTime))))
                    result = false;
                else if(FAILED(pDateTime->put_Value(vtProp.bstrVal)))
                    result = false;
                else if(FAILED(pDateTime->GetFileTime(VARIANT_TRUE, &dateStr)))
                    result = false;
                else
                    ffBstrToStrbuf(dateStr, strbuf);
                break;
            };

            default: result = false; break;
        }
    }
    VariantClear(&vtProp);
    return result;
}

bool ffGetWmiObjInteger(IWbemClassObject* obj, const wchar_t* key, int64_t* integer)
{
    bool result = true;

    VARIANT vtProp;
    VariantInit(&vtProp);

    CIMTYPE type;
    if(FAILED(obj->Get(key, 0, &vtProp, &type, nullptr)))
    {
        result = false;
    }
    else
    {
        switch(type)
        {
            case CIM_SINT8: *integer = vtProp.cVal; break;
            case CIM_SINT16: *integer = vtProp.iVal; break;
            case CIM_SINT32: *integer = vtProp.intVal; break;
            case CIM_SINT64: *integer = vtProp.llVal; break;
            case CIM_UINT8: *integer = (int64_t)vtProp.bVal; break;
            case CIM_UINT16: *integer = (int64_t)vtProp.uiVal; break;
            case CIM_UINT32: *integer = (int64_t)vtProp.uintVal; break;
            case CIM_UINT64: *integer = (int64_t)vtProp.ullVal; break;
            default: result = false;
        }
    }
    VariantClear(&vtProp);
    return result;
}
