extern "C"
{
#include "bluetooth.h"
}
#include "util/windows/wmi.hpp"
#include "util/windows/unicode.hpp"

STDAPI InitVariantFromStringArray(_In_reads_(cElems) PCWSTR *prgsz, _In_ ULONG cElems, _Out_ VARIANT *pvar);

template <typename Fn>
struct on_scope_exit {
    on_scope_exit(Fn &&fn): _fn(std::move(fn)) {}
    ~on_scope_exit() { this->_fn(); }

private:
    Fn _fn;
};

extern "C"
const char* ffBluetoothDetectBattery(FFlist* devices)
{
    FFWmiQuery query(L"SELECT __PATH FROM Win32_PnPEntity WHERE Service = 'BthHFEnum'", nullptr, FFWmiNamespace::CIMV2);
    if(!query)
        return "Query WMI service failed";

    IWbemClassObject* pInParams = nullptr;
    on_scope_exit releaseInParams([&] { pInParams && pInParams->Release(); });
    {
        IWbemClassObject* pnpEntityClass = nullptr;

        if (FAILED(query.pService->GetObjectW(bstr_t(L"Win32_PnPEntity"), 0, nullptr, &pnpEntityClass, nullptr)))
            return "Failed to get PnP entity class";
        on_scope_exit releasePnpEntityClass([&] { pnpEntityClass && pnpEntityClass->Release(); });

        if (FAILED(pnpEntityClass->GetMethod(bstr_t(L"GetDeviceProperties"), 0, &pInParams, NULL)))
            return "Failed to get GetDeviceProperties method";

        VARIANT devicePropertyKeys;
        PCWSTR props[] = { L"{104EA319-6EE2-4701-BD47-8DDBF425BBE5} 2", L"DEVPKEY_Bluetooth_DeviceAddress" };

        if (FAILED(InitVariantFromStringArray(props, ARRAY_SIZE(props), &devicePropertyKeys)))
            return "Failed to init variant from string array";
        on_scope_exit releaseDevicePropertyKeys([&] { VariantClear(&devicePropertyKeys); });

        if (FAILED(pInParams->Put(L"devicePropertyKeys", 0, &devicePropertyKeys, CIM_FLAG_ARRAY | CIM_STRING)))
            return "Failed to put devicePropertyKeys";
    }

    while (FFWmiRecord record = query.next())
    {
        IWbemCallResult* pCallResult = nullptr;

        if (FAILED(query.pService->ExecMethod(record.get(L"__PATH").bstrVal, bstr_t(L"GetDeviceProperties"), 0, nullptr, pInParams, nullptr, &pCallResult)))
            continue;
        on_scope_exit releaseCallResult([&] { pCallResult && pCallResult->Release(); });

        IWbemClassObject* pResultObject = nullptr;
        if (FAILED(pCallResult->GetResultObject((LONG) WBEM_INFINITE, &pResultObject)))
            continue;
        on_scope_exit releaseResultObject([&] { pResultObject && pResultObject->Release(); });

        VARIANT propArray;
        if (FAILED(pResultObject->Get(L"deviceProperties", 0, &propArray, nullptr, nullptr)))
            continue;
        on_scope_exit releasePropArray([&] { VariantClear(&propArray); });

        if (propArray.vt != (VT_ARRAY | VT_UNKNOWN) ||
            (propArray.parray->fFeatures & FADF_UNKNOWN) == 0 ||
            propArray.parray->cDims != 1 ||
            propArray.parray->rgsabound[0].cElements != 2
        )
            continue;

        uint8_t batt = 0;
        for (LONG i = 0; i < 2; i++)
        {
            IWbemClassObject* object = nullptr;
            if (FAILED(SafeArrayGetElement(propArray.parray, &i, &object)))
                continue;

            FFWmiRecord rec(object);
            auto data = rec.get(L"Data");
            if (data.vt == VT_EMPTY)
                break;

            if (i == 0)
                batt = data.get<uint8_t>();
            else
            {
                FF_STRBUF_AUTO_DESTROY addr; // MAC address without colon
                ffStrbufInitWSV(&addr, data.get<std::wstring_view>());
                if (__builtin_expect(addr.length != 12, 0))
                    continue;

                FF_LIST_FOR_EACH(FFBluetoothResult, bt, *devices)
                {
                    if (bt->address.length != 12 + 5)
                        continue;

                    if (addr.chars[0] == bt->address.chars[0] &&
                        addr.chars[1] == bt->address.chars[1] &&
                        addr.chars[2] == bt->address.chars[3] &&
                        addr.chars[3] == bt->address.chars[4] &&
                        addr.chars[4] == bt->address.chars[6] &&
                        addr.chars[5] == bt->address.chars[7] &&
                        addr.chars[6] == bt->address.chars[9] &&
                        addr.chars[7] == bt->address.chars[10] &&
                        addr.chars[8] == bt->address.chars[12] &&
                        addr.chars[9] == bt->address.chars[13] &&
                        addr.chars[10] == bt->address.chars[15] &&
                        addr.chars[11] == bt->address.chars[16])
                    {
                        bt->battery = batt;
                        break;
                    }
                }
            }
        }
    }

    return NULL;
}
