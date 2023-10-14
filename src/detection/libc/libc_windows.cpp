extern "C"
{
#include "libc.h"
}

#ifdef __MINGW32__
#include <_mingw.h>
#endif

template<uint32_t Major, uint32_t Minor>
class version_t {
    constexpr static auto buflen() noexcept {
        unsigned int len = 2; // "."
        if (Major == 0)
            len++;
        else
            for (auto n = Major; n; len++, n /= 10);

        if (Minor == 0)
            len++;
        else
            for (auto n = Minor; n; len++, n /= 10);
        return len;
    }

    char buf[buflen()] = {};

public:
    constexpr version_t() noexcept {
        auto ptr = buf + buflen();
        *--ptr = '\0';

        if (Minor == 0) {
            *--ptr = '0';
        } else {
            for (auto n = Minor; n; n /= 10)
                *--ptr = "0123456789"[n % 10];
        }
        *--ptr = '.';
        if (Major == 0) {
            *--ptr = '0';
        } else {
            for (auto n = Major; n; n /= 10)
                *--ptr = "0123456789"[n % 10];
        }
    }

    constexpr operator const char *() const { return buf; }
};

template<uint32_t Major, uint32_t Minor>
constexpr version_t<Major, Minor> version;

extern "C"
const char* ffDetectLibc(FFLibcResult* result)
{
#ifdef _UCRT
    result->name = "ucrt";
#else
    result->name = "msvcrt";
#endif

    result->version = version<(__MSVCRT_VERSION__ >> 8), (__MSVCRT_VERSION__ & 8)>;
    return NULL;
}
