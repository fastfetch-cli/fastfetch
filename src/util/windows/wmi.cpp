#include "wmi.hpp"

#include <synchapi.h>
#include <wchar.h>
#include <math.h>

namespace
{
    // Provide our bstr_t to avoid libstdc++ dependency
    struct bstr_t
    {
        explicit bstr_t(const wchar_t* str) noexcept: _bstr(SysAllocString(str)) {}
        ~bstr_t() noexcept { SysFreeString(_bstr); }
        explicit operator const wchar_t*() const noexcept { return _bstr; }
        operator BSTR() const noexcept { return _bstr; }

        private:
            BSTR _bstr;
    };
}

//https://learn.microsoft.com/en-us/windows/win32/wmisdk/example--getting-wmi-data-from-the-local-computer
//https://learn.microsoft.com/en-us/windows/win32/cimwin32prov/computer-system-hardware-classes
static void CoUninitializeWrap()
{
    CoUninitialize();
}

static BOOL CALLBACK InitHandleFunction(PINIT_ONCE, PVOID lpParameter, PVOID* lpContext)
{
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
        *((const char**)lpContext) = "Failed to initialize security";
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
        *((const char**)lpContext) = "Failed to create IWbemLocator object";
        return FALSE;
    }

    // Connect to WMI through the IWbemLocator::ConnectServer method
    IWbemServices* pSvc = nullptr;

    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
        bstr_t((const wchar_t*) lpParameter), // Object path of WMI namespace
        nullptr,                              // User name. nullptr = current user
        nullptr,                              // User password. nullptr = current
        0,                                    // Locale. nullptr indicates current
        0,                                    // Security flags.
        0,                                    // Authority (for example, Kerberos)
        0,                                    // Context object
        &pSvc                                 // pointer to IWbemServices proxy
    );
    pLoc->Release();
    pLoc = nullptr;

    if (FAILED(hres))
    {
        CoUninitialize();
        *((const char**)lpContext) = "Could not connect WMI server";
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
        *((const char**)lpContext) = "Could not set proxy blanket";
        return FALSE;
    }

    *((IWbemServices**)lpContext) = pSvc;
    atexit(CoUninitializeWrap);
    return TRUE;
}

FFWmiQuery::FFWmiQuery(const wchar_t* queryStr, FFstrbuf* error, FFWmiNamespace wmiNs)
    : pEnumerator(nullptr)
{
    static INIT_ONCE s_InitOnce[(int) FFWmiNamespace::LAST] = {};
    const char* context;
    if (InitOnceExecuteOnce(
        &s_InitOnce[(int)wmiNs],
        &InitHandleFunction,
        (PVOID) (wmiNs == FFWmiNamespace::CIMV2 ? L"ROOT\\CIMV2" : L"ROOT\\WMI"),
        (void**)&context) == FALSE
    ) {
        if(error)
            ffStrbufAppendS(error, context);
        return;
    }

    // Use the IWbemServices pointer to make requests of WMI
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
            ffStrbufAppendF(error, "Query for '%ls' failed. Error code = 0x%lX", queryStr, hres);
    }
}

static void ffBstrToStrbuf(BSTR bstr, FFstrbuf* strbuf)
{
    int len = (int)SysStringLen(bstr);
    if(len <= 0)
    {
        ffStrbufClear(strbuf);
        return;
    }
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, bstr, len, nullptr, 0, nullptr, nullptr);
    ffStrbufEnsureFree(strbuf, (uint32_t)size_needed);
    WideCharToMultiByte(CP_UTF8, 0, bstr, len, strbuf->chars, size_needed, nullptr, nullptr);
    strbuf->length = (uint32_t)size_needed;
    strbuf->chars[size_needed] = '\0';
}

bool FFWmiRecord::getString(const wchar_t* key, FFstrbuf* strbuf)
{
    bool result = true;

    VARIANT vtProp;
    VariantInit(&vtProp);

    CIMTYPE type;
    if(FAILED(obj->Get(key, 0, &vtProp, &type, nullptr)) || vtProp.vt != VT_BSTR)
    {
        result = false;
    }
    else
    {
        switch(vtProp.vt)
        {
            case VT_BSTR:
                if(type == CIM_DATETIME)
                {
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
                }
                else
                {
                    ffBstrToStrbuf(vtProp.bstrVal, strbuf);
                }
                break;

            case VT_LPSTR:
                ffStrbufAppendS(strbuf, vtProp.pcVal);
                break;

            case VT_LPWSTR: // TODO
            default:
                result = false;
                break;
        }
    }
    VariantClear(&vtProp);
    return result;
}

bool FFWmiRecord::getSigned(const wchar_t* key, int64_t* integer)
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
        switch(vtProp.vt)
        {
            case VT_BSTR: *integer = wcstoll(vtProp.bstrVal, nullptr, 10); break;
            case VT_I1: *integer = vtProp.cVal; break;
            case VT_I2: *integer = vtProp.iVal; break;
            case VT_INT:
            case VT_I4: *integer = vtProp.intVal; break;
            case VT_I8: *integer = vtProp.llVal; break;
            case VT_UI1: *integer = (int64_t)vtProp.bVal; break;
            case VT_UI2: *integer = (int64_t)vtProp.uiVal; break;
            case VT_UINT:
            case VT_UI4: *integer = (int64_t)vtProp.uintVal; break;
            case VT_UI8: *integer = (int64_t)vtProp.ullVal; break;
            case VT_BOOL: *integer = vtProp.boolVal != VARIANT_FALSE; break;
            default: *integer = 0; result = false;
        }
    }
    VariantClear(&vtProp);
    return result;
}

bool FFWmiRecord::getUnsigned(const wchar_t* key, uint64_t* integer)
{
    bool result = true;

    VARIANT vtProp;
    VariantInit(&vtProp);

    if(FAILED(obj->Get(key, 0, &vtProp, nullptr, nullptr)))
    {
        result = false;
    }
    else
    {
        switch(vtProp.vt)
        {
            case VT_BSTR: *integer = wcstoull(vtProp.bstrVal, nullptr, 10); break;
            case VT_I1: *integer = (uint64_t)vtProp.cVal; break;
            case VT_I2: *integer = (uint64_t)vtProp.iVal; break;
            case VT_INT:
            case VT_I4: *integer = (uint64_t)vtProp.intVal; break;
            case VT_I8: *integer = (uint64_t)vtProp.llVal; break;
            case VT_UI1: *integer = vtProp.bVal; break;
            case VT_UI2: *integer = vtProp.uiVal; break;
            case VT_UINT:
            case VT_UI4: *integer = vtProp.uintVal; break;
            case VT_UI8: *integer = vtProp.ullVal; break;
            case VT_BOOL: *integer = vtProp.boolVal != VARIANT_FALSE; break;
            default: *integer = 0; result = false;
        }
    }
    VariantClear(&vtProp);
    return result;
}

bool FFWmiRecord::getReal(const wchar_t* key, double* real)
{
    bool result = true;

    VARIANT vtProp;
    VariantInit(&vtProp);

    if(FAILED(obj->Get(key, 0, &vtProp, nullptr, nullptr)))
    {
        result = false;
    }
    else
    {
        switch(vtProp.vt)
        {
            case VT_BSTR: *real = wcstod(vtProp.bstrVal, nullptr); break;
            case VT_I1: *real = vtProp.cVal; break;
            case VT_I2: *real = vtProp.iVal; break;
            case VT_INT:
            case VT_I4: *real = vtProp.intVal; break;
            case VT_I8: *real = (double)vtProp.llVal; break;
            case VT_UI1: *real = vtProp.bVal; break;
            case VT_UI2: *real = vtProp.uiVal; break;
            case VT_UINT:
            case VT_UI4: *real = vtProp.uintVal; break;
            case VT_UI8: *real = (double)vtProp.ullVal; break;
            case VT_R4: *real = vtProp.fltVal; break;
            case VT_R8: *real = vtProp.dblVal; break;
            case VT_BOOL: *real = vtProp.boolVal != VARIANT_FALSE; break;
            default: *real = NAN; result = false;
        }
    }
    VariantClear(&vtProp);
    return result;
}
