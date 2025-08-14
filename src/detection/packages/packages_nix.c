#include "packages.h"
#include "common/io/io.h"
#include "common/processing.h"
#include "util/stringUtils.h"

static bool isValidNixPkg(FFstrbuf* pkg)
{
    if (!ffPathExists(pkg->chars, FF_PATHTYPE_DIRECTORY))
        return false;

    ffStrbufSubstrAfterLastC(pkg, '/');
    if (
        ffStrbufStartsWithS(pkg, "nixos-system-nixos-") ||
        ffStrbufEndsWithS(pkg, "-doc") ||
        ffStrbufEndsWithS(pkg, "-man") ||
        ffStrbufEndsWithS(pkg, "-info") ||
        ffStrbufEndsWithS(pkg, "-dev") ||
        ffStrbufEndsWithS(pkg, "-bin")
    ) return false;

    enum { START, DIGIT, DOT, MATCH } state = START;

    for (uint32_t i = 0; i < pkg->length; i++)
    {
        char c = pkg->chars[i];
        switch (state)
        {
            case START:
                if (ffCharIsDigit(c))
                    state = DIGIT;
                break;
            case DIGIT:
                if (ffCharIsDigit(c))
                    continue;
                if (c == '.')
                    state = DOT;
                else
                    state = START;
                break;
            case DOT:
                if (ffCharIsDigit(c))
                    state = MATCH;
                else
                    state = START;
                break;
            case MATCH:
                break;
        }
    }

    return state == MATCH;
}

static bool checkNixCache(FFstrbuf* cacheDir, FFstrbuf* hash, uint32_t* count)
{
    if (!ffPathExists(cacheDir->chars, FF_PATHTYPE_FILE))
        return false;

    FF_STRBUF_AUTO_DESTROY cacheContent = ffStrbufCreate();
    if (!ffReadFileBuffer(cacheDir->chars, &cacheContent))
        return false;

    // Format: <hash>\n<count>
    uint32_t split = ffStrbufFirstIndexC(&cacheContent, '\n');
    if (split == cacheContent.length)
        return false;

    ffStrbufSetNS(hash, split, cacheContent.chars);
    *count = (uint32_t)atoi(cacheContent.chars + split + 1);

    return true;
}

static bool writeNixCache(FFstrbuf* cacheDir, FFstrbuf* hash, uint32_t count)
{
    FF_STRBUF_AUTO_DESTROY cacheContent = ffStrbufCreateCopy(hash);
    ffStrbufAppendF(&cacheContent, "\n%u", count);
    return ffWriteFileBuffer(cacheDir->chars, &cacheContent);
}

static uint32_t getNixPackagesImpl(char* path)
{
    //Nix detection is kinda slow, so we only do it if the dir exists
    if(!ffPathExists(path, FF_PATHTYPE_DIRECTORY))
        return 0;

    FF_STRBUF_AUTO_DESTROY cacheDir = ffStrbufCreateCopy(&instance.state.platform.cacheDir);
    ffStrbufEnsureEndsWithC(&cacheDir, '/');
    ffStrbufAppendS(&cacheDir, "fastfetch/packages/nix");
    ffStrbufAppendS(&cacheDir, path);

    //Check the hash first to determine if we need to recompute the count
    FF_STRBUF_AUTO_DESTROY hash = ffStrbufCreateA(64);
    FF_STRBUF_AUTO_DESTROY cacheHash = ffStrbufCreateA(64);
    uint32_t count = 0;

    ffProcessAppendStdOut(&hash, (char* const[]) {
        "nix-store",
        "--query",
        "--hash",
        path,
        NULL
    });

    if (checkNixCache(&cacheDir, &cacheHash, &count) && ffStrbufEqual(&hash, &cacheHash))
        return count;

    //Cache is invalid, recompute the count
    count = 0;

    //Implementation based on bash script from here:
    //https://github.com/fastfetch-cli/fastfetch/issues/195#issuecomment-1191748222

    FF_STRBUF_AUTO_DESTROY output = ffStrbufCreateA(1024);

    ffProcessAppendStdOut(&output, (char* const[]) {
        "nix-store",
        "--query",
        "--requisites",
        path,
        NULL
    });

    uint32_t lineLength = 0;
    for (uint32_t i = 0; i < output.length; i++)
    {
        if (output.chars[i] != '\n')
        {
            lineLength++;
            continue;
        }

        output.chars[i] = '\0';
        FFstrbuf line = {
            .allocated = 0,
            .length = lineLength,
            .chars = output.chars + i - lineLength
        };
        if (isValidNixPkg(&line))
            count++;
        lineLength = 0;
    }

    writeNixCache(&cacheDir, &hash, count);
    return count;
}

uint32_t ffPackagesGetNix(FFstrbuf* baseDir, const char* dirname)
{
    uint32_t baseDirLength = baseDir->length;
    ffStrbufAppendS(baseDir, dirname);
    uint32_t num_elements = getNixPackagesImpl(baseDir->chars);
    ffStrbufSubstrBefore(baseDir, baseDirLength);
    return num_elements;
}
