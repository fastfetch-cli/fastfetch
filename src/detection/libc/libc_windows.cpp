extern "C" {
#include "libc.h"
}

#ifdef __MINGW32__
    #include <_mingw.h>
#endif

template <uint32_t Major, uint32_t Minor>
struct version_t {
    static constexpr uint32_t digits(uint32_t n) noexcept {
        uint32_t len = 1;
        for (auto temp = n / 10; temp; temp /= 10) {
            len++;
        }
        return len;
    }

    char buf[digits(Major) + digits(Minor) + 2];

    constexpr version_t() noexcept {
        auto ptr = buf + sizeof(buf);
        *--ptr = '\0';

        auto append = [&](uint32_t n) noexcept {
            if (n == 0) {
                *--ptr = '0';
            } else {
                for (; n; n /= 10) {
                    *--ptr = static_cast<char>('0' + n % 10);
                }
            }
        };

        append(Minor);
        *--ptr = '.';
        append(Major);
    }

    constexpr operator const char*() const noexcept {
        return buf;
    }
};

template <uint32_t Major, uint32_t Minor>
constexpr version_t<Major, Minor> version;

extern "C" const char* ffDetectLibc(FFLibcResult* result) {
#ifdef _UCRT
    result->name = "ucrt";
#else
    result->name = "msvcrt";
#endif

    result->version = version<(__MSVCRT_VERSION__ >> 8), (__MSVCRT_VERSION__ & 0xFF)>;
    return nullptr;
}
