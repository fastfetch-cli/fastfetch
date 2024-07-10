#include "base64.h"

// https://github.com/kostya/benchmarks/blob/master/base64/test-nolib.c#L145
void ffBase64EncodeRaw(uint32_t size, const char *str, uint32_t *out_size, char *output)
{
    static const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *out = output;
    const char *ends = str + (size - size % 3);
    while (str != ends)
    {
        uint32_t n = __builtin_bswap32(*(uint32_t *)str);
        *out++ = chars[(n >> 26) & 63];
        *out++ = chars[(n >> 20) & 63];
        *out++ = chars[(n >> 14) & 63];
        *out++ = chars[(n >> 8) & 63];
        str += 3;
    }

    if (size % 3 == 1)
    {
        uint64_t n = (uint64_t)*str << 16;
        *out++ = chars[(n >> 18) & 63];
        *out++ = chars[(n >> 12) & 63];
        *out++ = '=';
        *out++ = '=';
    }
    else if (size % 3 == 2)
    {
        uint64_t n = (uint64_t)*str++ << 16;
        n |= (uint64_t)*str << 8;
        *out++ = chars[(n >> 18) & 63];
        *out++ = chars[(n >> 12) & 63];
        *out++ = chars[(n >> 6) & 63];
        *out++ = '=';
    }
    *out = '\0';
    *out_size = (uint32_t)(out - output);
}

static uint8_t decode_table[256];

void init_decode_table()
{
    uint8_t ch = 0;
    do
    {
        int32_t code = -1;
        if (ch >= 'A' && ch <= 'Z')
            code = ch - 0x41;
        if (ch >= 'a' && ch <= 'z')
            code = ch - 0x47;
        if (ch >= '0' && ch <= '9')
            code = ch + 0x04;
        if (ch == '+' || ch == '-')
            code = 0x3E;
        if (ch == '/' || ch == '_')
            code = 0x3F;
        decode_table[ch] = (uint8_t) code;
    } while (ch++ < 0xFF);
}

#define next_char(x) uint8_t x = decode_table[(uint8_t) *str++];

bool ffBase64DecodeRaw(uint32_t size, const char *str, uint32_t *out_size, char *output)
{
    if (*(uint64_t*) decode_table == 0)
        init_decode_table();

    char *out = output;
    while (size > 0 && (str[size - 1] == '\n' || str[size - 1] == '\r' || str[size - 1] == '='))
        size--;

    const char *ends = str + size - 4;
    while (true)
    {
        if (str > ends)
            break;
        while (*str == '\n' || *str == '\r')
            str++;

        if (str > ends)
            break;
        next_char(a);
        next_char(b);
        next_char(c);
        next_char(d);

        *out++ = (char)(a << 2 | b >> 4);
        *out++ = (char)(b << 4 | c >> 2);
        *out++ = (char)(c << 6 | d >> 0);
    }

    uint8_t mod = (uint8_t) (ends - str + 4) % 4;
    if (mod == 2)
    {
        next_char(a);
        next_char(b);
        *out++ = (char)(a << 2 | b >> 4);
    }
    else if (mod == 3)
    {
        next_char(a);
        next_char(b);
        next_char(c);
        *out++ = (char)(a << 2 | b >> 4);
        *out++ = (char)(b << 4 | c >> 2);
    }

    *out = '\0';
    *out_size = (uint32_t) (out - output);
    return true;
}
