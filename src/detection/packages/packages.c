#include "packages.h"
#include "common/io.h"

#include <inttypes.h>
#include <stddef.h>

void ffDetectPackagesImpl(FFPackagesResult* result, FFPackagesOptions* options);

const char* ffDetectPackages(FFPackagesResult* result, FFPackagesOptions* options) {
    ffDetectPackagesImpl(result, options);

    for (uint32_t i = 0; i < offsetof(FFPackagesResult, all) / sizeof(uint32_t); ++i) {
        result->all += ((uint32_t*) result)[i];
    }

    return nullptr;
}

bool ffPackagesReadCacheKey(FFstrbuf* cacheDir, FFstrbuf* cacheContent, uint64_t cacheKey, const char* packageId, uint32_t* result) {
    if (__builtin_expect(cacheKey == 0, false)) {
        // The caller could not compute a cache key (its data source is unreadable). Nothing may be
        // cached under an unknown key, and reporting 0 would be a lie. Leaving `cacheContent` empty
        // makes the matching ffPackagesWriteCache() call a no-op.
        return false;
    }

    ffStrbufSet(cacheDir, &instance.state.platform.cacheDir);
    ffStrbufEnsureEndsWithC(cacheDir, '/');
    ffStrbufAppendF(cacheDir, "fastfetch/packages/%s.txt", packageId);

    if (ffReadFileBuffer(cacheDir->chars, cacheContent)) {
        uint64_t key_cached;
        uint32_t num_cached;
        if (sscanf(cacheContent->chars, "%" SCNu64 " %" SCNu32, &key_cached, &num_cached) == 2 &&
            key_cached == cacheKey && num_cached > 0) {
            *result = num_cached;
            return true;
        }
    }

    ffStrbufSetF(cacheContent, "%" PRIu64 " ", cacheKey);

    return false;
}

bool ffPackagesReadCache(FFstrbuf* cacheDir, FFstrbuf* cacheContent, const char* filePath, const char* packageId, uint32_t* result) {
    const uint64_t mtime_current = ffPathGetMtime(filePath);
    if (__builtin_expect(mtime_current == 0, false)) {
        // A missing database legitimately means "no packages installed", and must not be cached.
        // A database whose modification time could not be read must not be cached either, and
        // reporting 0 for it would be a lie. ffPathGetMtime() reports 0 for both, so tell them
        // apart before deciding which of the two applies.
        if (ffPathExists(filePath, FF_PATHTYPE_FILE)) {
            return false;
        }

        *result = 0;
        return true;
    }

    return ffPackagesReadCacheKey(cacheDir, cacheContent, mtime_current, packageId, result);
}

bool ffPackagesWriteCache(FFstrbuf* cacheDir, FFstrbuf* cacheContent, uint32_t num_elements) {
    if (__builtin_expect(cacheContent->length == 0, false)) {
        return false;
    }

    ffStrbufAppendF(cacheContent, "%" PRIu32, num_elements);
    return ffWriteFileBuffer(cacheDir->chars, cacheContent);
}

#if __linux__
#include <sys/syscall.h>

struct linux_dirent64 {
    uint64_t       d_ino;    /* 64-bit inode number */
    int64_t        d_off;    /* Not an offset; see getdents() */
    unsigned short d_reclen; /* Size of this dirent */
    unsigned char  d_type;   /* File type */
    char           d_name[]; /* Filename (null-terminated) */
};

uint32_t ffPackagesGetNumElements(const char* dirname, bool isdir) {
    FF_AUTO_CLOSE_FD int fd = open(dirname, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (fd < 0) {
        return 0;
    }

    alignas(struct linux_dirent64) uint8_t bytes[64 * 1024];

    uint32_t num_elements = 0;
    const size_t nameOffset = offsetof(struct linux_dirent64, d_name);

    for (;;) {
        long bytesRead = syscall(SYS_getdents64, fd, bytes, sizeof(bytes));
        if (bytesRead < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        if (bytesRead == 0) {
            break;
        }

        size_t remaining = (size_t) bytesRead;
        struct linux_dirent64* entry = (struct linux_dirent64*) bytes;

        while (remaining >= nameOffset + 1) {
            bool ok = false;
            if (entry->d_name[0] != '.') {
                if (__builtin_expect(entry->d_type != DT_UNKNOWN && entry->d_type != DT_LNK, true)) {
                    ok = entry->d_type == (isdir ? DT_DIR : DT_REG);
                } else {
                    struct stat stbuf;
                    if (fstatat(fd, entry->d_name, &stbuf, 0) == 0) {
                        ok = isdir ? S_ISDIR(stbuf.st_mode) : S_ISREG(stbuf.st_mode);
                    }
                }
            }

            num_elements += ok;
            size_t recordLength = entry->d_reclen;
            remaining -= recordLength;
            entry = (struct linux_dirent64*) ((uint8_t*) entry + recordLength);
        }

        if (remaining != 0) {
            break;
        }
    }

    return num_elements;
}

#elif !_WIN32
uint32_t ffPackagesGetNumElements(const char* dirname, bool isdir) {
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(dirname);
    if (dirp == nullptr) {
        return 0;
    }

    uint32_t num_elements = 0;

    struct dirent* entry;
    while ((entry = readdir(dirp)) != nullptr) {
        bool ok = false;

        if (entry->d_name[0] != '.') {
    #if !defined(__sun) && !defined(__HAIKU__)
            if (entry->d_type != DT_UNKNOWN && entry->d_type != DT_LNK) {
                ok = entry->d_type == (isdir ? DT_DIR : DT_REG);
            } else
    #endif
            {
                struct stat stbuf;
                if (fstatat(dirfd(dirp), entry->d_name, &stbuf, 0) == 0) {
                    ok = isdir ? S_ISDIR(stbuf.st_mode) : S_ISREG(stbuf.st_mode);
                }
            }
        }

        if (ok) {
            ++num_elements;
        }
    }

    return num_elements;
}
#endif
