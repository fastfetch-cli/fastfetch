#include "registry.h"
#include "unicode.h"
#include "common/mallocHelper.h"
#include "common/debug.h"
#include "common/windows/nt.h"

#include <stdalign.h>
#include <ntstatus.h>

static HANDLE hRootKeys[8 /*(uintptr_t) HKEY_CURRENT_USER_LOCAL_SETTINGS - (uintptr_t) HKEY_CLASSES_ROOT + 1*/];

static const char* hKey2Str(HANDLE hRootKey) {
#define HKEY_CASE(compareKey) \
    if (hRootKey == hRootKeys[(uintptr_t) compareKey - (uintptr_t) HKEY_CLASSES_ROOT]) return #compareKey;
    HKEY_CASE(HKEY_CLASSES_ROOT)
    HKEY_CASE(HKEY_CURRENT_USER)
    HKEY_CASE(HKEY_LOCAL_MACHINE)
    HKEY_CASE(HKEY_USERS)
    HKEY_CASE(HKEY_PERFORMANCE_DATA)
    HKEY_CASE(HKEY_CURRENT_CONFIG)
    HKEY_CASE(HKEY_DYN_DATA)
    HKEY_CASE(HKEY_CURRENT_USER_LOCAL_SETTINGS)
#undef HKEY_CASE

    return "UNKNOWN";
}

HANDLE ffRegGetRootKeyHandle(HKEY hKey) {
    assert(hKey);
    assert((uintptr_t) hKey >= (uintptr_t) HKEY_CLASSES_ROOT && (uintptr_t) hKey <= (uintptr_t) HKEY_CURRENT_USER_LOCAL_SETTINGS);

    FF_DEBUG("Getting root key handle for HKEY %08llx", (uint64_t) (uintptr_t) hKey);

    HANDLE result = hRootKeys[(uintptr_t) hKey - (uintptr_t) HKEY_CLASSES_ROOT];
    if (result) {
        FF_DEBUG("Found cached root key handle for %s -> %p", hKey2Str(result), result);
        return result;
    }

    switch ((uintptr_t) hKey) {
        case (uintptr_t) HKEY_CURRENT_USER: {
            NTSTATUS status = RtlOpenCurrentUser(KEY_READ, &result);
            if (!NT_SUCCESS(status)) {
                FF_DEBUG("RtlOpenCurrentUser() failed: %s", ffDebugNtStatus(status));
                return NULL;
            }
            break;
        }

        case (uintptr_t) HKEY_LOCAL_MACHINE: {
            UNICODE_STRING path = RTL_CONSTANT_STRING(L"\\Registry\\Machine");
            NTSTATUS status = NtOpenKey(&result, KEY_READ, &(OBJECT_ATTRIBUTES) {
                                                               .Length = sizeof(OBJECT_ATTRIBUTES),
                                                               .RootDirectory = NULL,
                                                               .ObjectName = &path,
                                                               .Attributes = OBJ_CASE_INSENSITIVE,
                                                           });
            if (!NT_SUCCESS(status)) {
                FF_DEBUG("NtOpenKey(%ls) failed: %s (0x%08lx)", path.Buffer, ffDebugNtStatus(status), status);
                return NULL;
            }
            break;
        }
        default:
            // Unsupported
            FF_DEBUG("Unsupported root key: %p", hKey);
            assert(false);
            return NULL;
    }
    hRootKeys[(uintptr_t) hKey - (uintptr_t) HKEY_CLASSES_ROOT] = result;
    FF_DEBUG("Opened root key %s -> %p", hKey2Str(result), result);
    return result;
}

bool ffRegOpenSubkeyForRead(HANDLE hKey, const wchar_t* subKeyW, HANDLE* result, FFstrbuf* error) {
    assert(hKey);
    assert(subKeyW);
    assert(result);

    FF_DEBUG("Opening subkey %s\\%ls for read", hKey2Str(hKey), subKeyW);

    USHORT subKeyLen = (USHORT) (wcslen(subKeyW) * sizeof(wchar_t));
    if (!NT_SUCCESS(NtOpenKey(result, KEY_READ, &(OBJECT_ATTRIBUTES) {
                                                    .Length = sizeof(OBJECT_ATTRIBUTES),
                                                    .RootDirectory = hKey,
                                                    .ObjectName = &(UNICODE_STRING) {
                                                        .Length = subKeyLen,
                                                        .MaximumLength = subKeyLen + (USHORT) sizeof(wchar_t),
                                                        .Buffer = (wchar_t*) subKeyW,
                                                    },
                                                }))) {
        FF_DEBUG("NtOpenKey(%s\\<subkey>) failed", hKey2Str(hKey));
        if (error) {
            FF_STRBUF_AUTO_DESTROY subKeyA = ffStrbufCreateWS(subKeyW);
            ffStrbufAppendF(error, "NtOpenKey(%s\\%s) failed", hKey2Str(hKey), subKeyA.chars);
        }
        return false;
    }
    FF_DEBUG("Opened subkey under %s -> %p", hKey2Str(hKey), *result);
    return true;
}

static bool processRegValue(const FFRegValueArg* arg, const ULONG regType, const void* regData, ULONG regDataLen, FFstrbuf* error) {
    switch (arg->type) {
        case FF_ARG_TYPE_STRBUF: {
            if (regType != REG_SZ && regType != REG_EXPAND_SZ) {
                goto type_mismatch;
            }

            FFstrbuf* strbuf = (FFstrbuf*) arg->value;
            uint32_t strLen = regDataLen / sizeof(wchar_t);
            if (strLen == 0) {
                ffStrbufClear(strbuf);
            } else {
                const wchar_t* ws = (const wchar_t*) regData;
                if (ws[strLen - 1] == L'\0') {
                    --strLen;
                }
                ffStrbufSetNWS(strbuf, strLen, ws);
            }
            break;
        }

        case FF_ARG_TYPE_UINT:
        case FF_ARG_TYPE_UINT64:
        case FF_ARG_TYPE_UINT16:
        case FF_ARG_TYPE_UINT8:
        case FF_ARG_TYPE_BOOL: {
            uint64_t value = 0;

            if (regType == REG_DWORD) {
                if (regDataLen < sizeof(uint32_t)) {
                    goto type_mismatch;
                }
                value = *(uint32_t*) regData;
            } else if (regType == REG_QWORD) {
                if (regDataLen < sizeof(uint64_t)) {
                    goto type_mismatch;
                }
                value = *(uint64_t*) regData;
            } else {
                goto type_mismatch;
            }

            if (arg->type == FF_ARG_TYPE_UINT) {
                *(uint32_t*) arg->value = (uint32_t) value;
            } else if (arg->type == FF_ARG_TYPE_UINT64) {
                *(uint64_t*) arg->value = (uint64_t) value;
            } else if (arg->type == FF_ARG_TYPE_UINT16) {
                *(uint16_t*) arg->value = (uint16_t) value;
            } else if (arg->type == FF_ARG_TYPE_UINT8) {
                *(uint8_t*) arg->value = (uint8_t) value;
            } else if (arg->type == FF_ARG_TYPE_BOOL) {
                *(bool*) arg->value = value != 0;
            }
            break;
        }

        case FF_ARG_TYPE_FLOAT: {
            if (regDataLen < sizeof(float)) {
                goto type_mismatch;
            }
            *(float*) arg->value = *(float*) regData;
            break;
        }

        case FF_ARG_TYPE_DOUBLE: {
            if (regDataLen < sizeof(double)) {
                goto type_mismatch;
            }
            *(double*) arg->value = *(double*) regData;
            break;
        }

        case FF_ARG_TYPE_LIST: {
            if (regType != REG_MULTI_SZ) {
                goto type_mismatch;
            }

            FFlist* list = (FFlist*) arg->value;
            ffListClear(list);

            for (
                const wchar_t* ptr = (const wchar_t*) regData;
                (const uint8_t*) ptr < (const uint8_t*) regData + regDataLen && *ptr;
                ptr++) {
                uint32_t strLen = (uint32_t) wcsnlen(ptr, regDataLen / sizeof(wchar_t) - (size_t) (ptr - (const wchar_t*) regData));
                ffStrbufInitNWS(FF_LIST_ADD(FFstrbuf, *list), strLen, ptr);
                ptr += strLen;
            }
            break;
        }

        case FF_ARG_TYPE_BUFFER: {
            if (regType != REG_BINARY) {
                goto type_mismatch;
            }

            FFArgBuffer* buffer = (FFArgBuffer*) arg->value;
            if (buffer->length == 0) {
                buffer->data = malloc(regDataLen);
            } else if (buffer->length < regDataLen) {
                if (error) {
                    FF_STRBUF_AUTO_DESTROY nameA = arg->name ? ffStrbufCreateWS(arg->name) : ffStrbufCreateStatic("(default)");
                    ffStrbufAppendF(error, "ffRegReadValues(%s) buffer too small (%u): expected %u", nameA.chars, (unsigned) buffer->length, (unsigned) regDataLen);
                }
                return false;
            }
            buffer->length = regDataLen;
            memcpy(buffer->data, regData, regDataLen);
            break;
        }

        case FF_ARG_TYPE_INT: // Use UINT instead
        case FF_ARG_TYPE_STRING:
        case FF_ARG_TYPE_NULL:
        default:
            if (error) {
                FF_STRBUF_AUTO_DESTROY nameA = arg->name ? ffStrbufCreateWS(arg->name) : ffStrbufCreateStatic("(default)");
                ffStrbufAppendF(error, "processRegValue(%s) unsupported FFArgType %u", nameA.chars, (unsigned) arg->type);
            }
            return false;
    }

    return true;

type_mismatch:
    FF_DEBUG("ffRegReadValues(%ls) type mismatch: regType=%u, argType=%u, dataLen=%u",
        arg->name ?: L"(default)",
        (unsigned) regType,
        (unsigned) arg->type,
        (unsigned) regDataLen);
    if (error) {
        FF_STRBUF_AUTO_DESTROY nameA = arg->name ? ffStrbufCreateWS(arg->name) : ffStrbufCreateStatic("(default)");
        ffStrbufAppendF(error, "ffRegReadValues(%s) type mismatch: regType=%u, argType=%u, dataLen=%u", nameA.chars, (unsigned) regType, (unsigned) arg->type, (unsigned) regDataLen);
    }
    return false;
}

bool ffRegReadValue(HANDLE hKey, const FFRegValueArg* arg, FFstrbuf* error) {
    UNICODE_STRING* valueNameU = &(UNICODE_STRING) {
        .Length = arg->name ? (USHORT) (wcslen(arg->name) * sizeof(wchar_t)) : 0 /*(default)*/,
        .MaximumLength = 0,
        .Buffer = (wchar_t*) arg->name,
    };

    alignas(KEY_VALUE_PARTIAL_INFORMATION) uint8_t staticBuffer[128 + sizeof(KEY_VALUE_PARTIAL_INFORMATION)];
    FF_AUTO_FREE uint8_t* dynamicBuffer = NULL;

    KEY_VALUE_PARTIAL_INFORMATION* buffer = (KEY_VALUE_PARTIAL_INFORMATION*) &staticBuffer;
    DWORD bufSize = sizeof(staticBuffer);
    if (NT_SUCCESS(NtQueryValueKey(hKey, valueNameU, KeyValuePartialInformation, buffer, bufSize, &bufSize))) {
        goto process_value;
    }

    if (bufSize == 0) {
        FF_DEBUG("NtQueryValueKey(%p, %ls) failed (bufSize=0)", hKey, arg->name ?: L"(default)");
        if (error) {
            FF_STRBUF_AUTO_DESTROY valueNameA = arg->name ? ffStrbufCreateWS(arg->name) : ffStrbufCreateStatic("(default)");
            ffStrbufAppendF(error, "NtQueryValueKey(%p, %s) failed", hKey, valueNameA.chars);
        }
        return false;
    }

    dynamicBuffer = (uint8_t*) malloc(bufSize);
    buffer = (KEY_VALUE_PARTIAL_INFORMATION*) dynamicBuffer;

    if (!NT_SUCCESS(NtQueryValueKey(hKey, valueNameU, KeyValuePartialInformation, buffer, bufSize, &bufSize))) {
        FF_DEBUG("NtQueryValueKey(%p, %ls, buffer=%u) failed", hKey, arg->name ?: L"(default)", (unsigned) bufSize);
        if (error) {
            FF_STRBUF_AUTO_DESTROY valueNameA = arg->name ? ffStrbufCreateWS(arg->name) : ffStrbufCreateStatic("(default)");
            ffStrbufAppendF(error, "NtQueryValueKey(%p, %s, buffer) failed", hKey, valueNameA.chars);
        }
        return false;
    }

process_value:
    FF_DEBUG("Read value from %p (%ls), type=%u, len=%u", hKey, arg->name ?: L"(default)", (unsigned) buffer->Type, (unsigned) buffer->DataLength);
    return processRegValue(arg, buffer->Type, buffer->Data, buffer->DataLength, error);
}

bool ffRegReadValues(HANDLE hKey, uint32_t argc, const FFRegValueArg argv[], FFstrbuf* error) {
    if (__builtin_expect(argc == 0, false)) {
        return true;
    }

    assert(argv);

    FF_AUTO_FREE UNICODE_STRING* names = (UNICODE_STRING*) calloc(argc, sizeof(*names));
    FF_AUTO_FREE KEY_VALUE_ENTRY* entries = (KEY_VALUE_ENTRY*) calloc(argc, sizeof(*entries));

    for (uint32_t i = 0; i < argc; ++i) {
        if (__builtin_expect(!argv[i].value, false)) {
            FF_DEBUG("ffRegReadValues(argv[%u].value) is NULL", (unsigned) i);
            if (error) {
                ffStrbufAppendF(error, "ffRegReadValues(argv[%u].pVar) is NULL", (unsigned) i);
            }
            return false;
        }

        names[i] = (UNICODE_STRING) {
            .Length = argv[i].name ? (USHORT) (wcslen(argv[i].name) * sizeof(wchar_t)) : 0 /*(default)*/,
            .MaximumLength = 0,
            .Buffer = (wchar_t*) argv[i].name,
        };
        entries[i].ValueName = &names[i];
    }

    ULONG bufferSize = argc * 128;
    if (bufferSize < 512) {
        bufferSize = 512;
    }

    FF_AUTO_FREE uint8_t* buffer = NULL;

    while (true) {
        buffer = (uint8_t*) realloc(buffer, bufferSize);

        ULONG writtenSize = bufferSize;
        ULONG requiredSize = 0;
        NTSTATUS status = NtQueryMultipleValueKey(hKey, entries, argc, buffer, &writtenSize, &requiredSize);

        if (!NT_SUCCESS(status)) {
            // Buffer too small: docs guarantee requiredSize is returned when provided.
            if (requiredSize > bufferSize) {
                FF_DEBUG("NtQueryMultipleValueKey(%p) resize buffer: %u -> %u", hKey, (unsigned) bufferSize, (unsigned) requiredSize);
                bufferSize = requiredSize;
                continue;
            }

            FF_DEBUG("NtQueryMultipleValueKey(%p, argc=%u) failed, status=0x%08X", hKey, (unsigned) argc, (unsigned) status);
            if (error) {
                ffStrbufAppendF(error, "NtQueryMultipleValueKey(%p, argc=%u) failed, status=0x%08X", hKey, (unsigned) argc, (unsigned) status);
            }
            return false;
        }

        break;
    }

    for (uint32_t i = 0; i < argc; ++i) {
        const FFRegValueArg* arg = &argv[i];
        const KEY_VALUE_ENTRY* entry = &entries[i];

        FF_DEBUG("Read value[%u] from %p: type=%u, len=%u", (unsigned) i, hKey, (unsigned) entry->Type, (unsigned) entry->DataLength);
        if (!processRegValue(arg, entry->Type, buffer + entry->DataOffset, entry->DataLength, error)) {
            return false;
        }
    }

    return true;
}

bool ffRegGetSubKey(HANDLE hKey, uint32_t index, FFstrbuf* result, FFstrbuf* error) {
    assert(hKey);
    assert(result);

    alignas(KEY_BASIC_INFORMATION) uint8_t buffer[sizeof(KEY_BASIC_INFORMATION) + MAX_PATH * sizeof(wchar_t)];
    ULONG bufSize = (ULONG) sizeof(buffer);
    KEY_BASIC_INFORMATION* keyInfo = (KEY_BASIC_INFORMATION*) buffer;

    if (!NT_SUCCESS(NtEnumerateKey(hKey, index, KeyBasicInformation, keyInfo, bufSize, &bufSize))) {
        FF_DEBUG("NtEnumerateKey(hKey=%p, index=%u) failed", hKey, (unsigned) index);
        if (error) {
            ffStrbufAppendF(error, "NtEnumerateKey(hKey=%p, %u, keyInfo) failed", hKey, (unsigned) index);
        }
        return false;
    }

    ffStrbufSetNWS(result, keyInfo->NameLength / sizeof(wchar_t), keyInfo->Name);
    return true;
}

bool ffRegGetNSubKeys(HANDLE hKey, uint32_t* result, FFstrbuf* error) {
    assert(hKey);
    assert(result);

    alignas(KEY_FULL_INFORMATION) uint8_t buffer[sizeof(KEY_FULL_INFORMATION) + MAX_PATH * sizeof(wchar_t)];
    ULONG bufSize = sizeof(buffer);
    KEY_FULL_INFORMATION* keyInfo = (KEY_FULL_INFORMATION*) buffer;

    if (!NT_SUCCESS(NtQueryKey(hKey, KeyFullInformation, keyInfo, bufSize, &bufSize))) {
        FF_DEBUG("NtQueryKey(hKey=%p, KeyFullInformation) failed", hKey);
        if (error) {
            ffStrbufAppendF(error, "NtQueryKey(hKey=%p, KeyFullInformation, keyInfo) failed", hKey);
        }
        return false;
    }

    *result = (uint32_t) keyInfo->SubKeys;
    return true;
}
