#include "getline.h"

#include <stdlib.h>
#include <errno.h>

ssize_t getline(char** lineptr, size_t* n, FILE* stream) {
    ssize_t pos = -1;
    int c;

    if (lineptr == nullptr || stream == nullptr || n == nullptr) {
        errno = EINVAL;
        return -1;
    }

    _lock_file(stream);

    c = _getc_nolock(stream);
    if (c == EOF) {
        goto exit;
    }

    if (*lineptr == nullptr) {
        *lineptr = malloc(128);
        if (*lineptr == nullptr) {
            goto exit;
        }
        *n = 128;
    }

    pos = 0;
    while (c != EOF) {
        if ((size_t) (pos + 1) >= *n) {
            size_t new_size = *n + (*n >> 2);
            if (new_size < 128) {
                new_size = 128;
            }
            char* new_ptr = realloc(*lineptr, new_size);
            if (new_ptr == nullptr) {
                pos = -1;
                goto exit;
            }
            *n = new_size;
            *lineptr = new_ptr;
        }

        ((char*) (*lineptr))[pos++] = (char) c;
        if (c == '\n') {
            break;
        }
        c = _getc_nolock(stream);
    }

    (*lineptr)[pos] = '\0';

exit:
    _unlock_file(stream);
    return pos;
}
