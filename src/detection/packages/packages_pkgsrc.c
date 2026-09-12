#include "packages.h"
#include "common/path.h"
#include "common/properties.h"

uint32_t ffPackagesGetPkgsrc(FFstrbuf* baseDir) {
    uint32_t baseDirLength = baseDir->length;
    FF_STRBUF_AUTO_DESTROY pkginPath = ffStrbufCreate();

    if (ffFindExecutableInPath("pkgin", &pkginPath) != NULL) {
        return 0;
    }

    ffStrbufSubstrBeforeLastC(&pkginPath, '/'); // remove /pkgin
    ffStrbufSubstrBeforeLastC(&pkginPath, '/'); // remove /bin
    uint32_t pkginPathLength = pkginPath.length;

    ffStrbufAppendS(&pkginPath, "/etc/mk.conf");

    FF_STRBUF_AUTO_DESTROY dbdir = ffStrbufCreate();
    ffParsePropFile(pkginPath.chars, "PKG_DBDIR=", &dbdir);
    ffStrbufSubstrBefore(&pkginPath, pkginPathLength); // remove /etc/mk.conf

    if (dbdir.length > 0) {
        ffStrbufAppend(baseDir, &dbdir);
    } else {
        ffStrbufAppend(baseDir, &pkginPath);
        ffStrbufAppendS(baseDir, "/pkgdb");
    }

    uint32_t result = ffPackagesGetNumElements(baseDir->chars, true);
    ffStrbufSubstrBefore(baseDir, baseDirLength);

    return result;
}
