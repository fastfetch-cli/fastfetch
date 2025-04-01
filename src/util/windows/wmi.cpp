#include "wmi.hpp"
#include "util/windows/com.hpp"
#include "util/windows/unicode.hpp"

#include <synchapi.h>
#include <wchar.h>
#include <math.h>

static const char* doInitService(const wchar_t* networkResource, IWbemServices** result)
{
    HRESULT hres;

    // Obtain the initial locator to WMI
    IWbemLocator* pLoc = nullptr;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*) &pLoc);

    if (FAILED(hres))
        return "Failed to create IWbemLocator object";

    // Connect to WMI through the IWbemLocator::ConnectServer method
    IWbemServices* pSvc = nullptr;

    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
        bstr_t(networkResource),              // Object path of WMI namespace
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
        return "Could not connect WMI server";

    *result = pSvc;
    return NULL;
}

FFWmiQuery::FFWmiQuery(const wchar_t* queryStr, FFstrbuf* error, FFWmiNamespace wmiNs)
{
    const char* errStr;
    if ((errStr = ffInitCom()))
    {
        if (error)
            ffStrbufSetS(error, errStr);
        return;
    }

    static IWbemServices* contexts[(int) FFWmiNamespace::LAST];

    IWbemServices* context = contexts[(int)wmiNs];
    if (!context)
    {
        if ((errStr = doInitService(wmiNs == FFWmiNamespace::CIMV2 ? L"ROOT\\CIMV2" : L"ROOT\\WMI", &context)))
        {
            if (error)
                ffStrbufSetS(error, errStr);
            return;
        }
        contexts[(int)wmiNs] = context;
    }

    this->pService = context;

    // Use the IWbemServices pointer to make requests of WMI
    HRESULT hres = context->ExecQuery(
        bstr_t(L"WQL"),
        bstr_t(queryStr),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        nullptr,
        &this->pEnumerator);

    if (FAILED(hres))
    {
        if(error)
            ffStrbufAppendF(error, "Query for '%ls' failed. Error code = 0x%lX", queryStr, hres);
    }
}

bool FFWmiRecord::getString(const wchar_t* key, FFstrbuf* strbuf)
{
    bool result = true;

    FFWmiVariant vtProp;

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
                    FF_AUTO_RELEASE_COM_OBJECT ISWbemDateTime *pDateTime = nullptr;
                    BSTR dateStr;
                    if(FAILED(CoCreateInstance(__uuidof(SWbemDateTime), 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDateTime))))
                        result = false;
                    else if(FAILED(pDateTime->put_Value(vtProp.bstrVal)))
                        result = false;
                    else if(FAILED(pDateTime->GetFileTime(VARIANT_TRUE, &dateStr)))
                        result = false;
                    else
                        ffStrbufSetNWS(strbuf, SysStringLen(dateStr), dateStr);
                }
                else
                {
                    ffStrbufSetNWS(strbuf, SysStringLen(vtProp.bstrVal), vtProp.bstrVal);
                }
                break;

            case VT_LPSTR:
                ffStrbufAppendS(strbuf, vtProp.pcVal);
                break;

            case VT_LPWSTR:
            default:
                ffStrbufSetWS(strbuf, vtProp.bstrVal);
                break;
        }
    }
    return result;
}

bool FFWmiRecord::getSigned(const wchar_t* key, int64_t* integer)
{
    bool result = true;

    FFWmiVariant vtProp;

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
    return result;
}

bool FFWmiRecord::getUnsigned(const wchar_t* key, uint64_t* integer)
{
    bool result = true;

    FFWmiVariant vtProp;

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
    return result;
}

bool FFWmiRecord::getReal(const wchar_t* key, double* real)
{
    bool result = true;

    FFWmiVariant vtProp;

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
    return result;
}
