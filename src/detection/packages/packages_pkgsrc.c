#include "packages.h"
#include "common/io.h"
#include "common/path.h"
#include "common/properties.h"

uint32_t ffPackagesGetPkgsrc(FFstrbuf* baseDir) {
    uint32_t baseDirLength = baseDir->length;
    FF_STRBUF_AUTO_DESTROY pkginPath = ffStrbufCreate();

    if (ffFindExecutableInPath("pkgin", &pkginPath) != NULL) {
        ffStrbufSubstrBefore(baseDir, baseDirLength);
        return 0;
    }

    ffStrbufSubstrBeforeLastC(&pkginPath, '/'); // remove /pkgin
    ffStrbufSubstrBeforeLastC(&pkginPath, '/'); // remove /bin

    FF_STRBUF_AUTO_DESTROY mkconf = ffStrbufCreateCopy(&pkginPath);
    ffStrbufAppendS(&mkconf, "/etc/mk.conf");

    FF_STRBUF_AUTO_DESTROY dbdir = ffStrbufCreate();
    ffParsePropFile(mkconf.chars, "PKG_DBDIR=", &dbdir);

    if (dbdir.length > 0) {
        ffStrbufAppendS(baseDir, dbdir.chars);
    } else {
        ffStrbufAppendF(baseDir, "%s/pkgdb", pkginPath.chars);
    }

    uint32_t result = ffPackagesGetNumElements(baseDir->chars, true);
    ffStrbufSubstrBefore(baseDir, baseDirLength);
    
    return result;
}
