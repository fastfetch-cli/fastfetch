#pragma once

#include "fastfetch.h"
#include "modules/packages/option.h"

typedef struct FFPackagesResult
{
    uint32_t amSystem;
    uint32_t amUser;
    uint32_t apk;
    uint32_t brew;
    uint32_t brewCask;
    uint32_t choco;
    uint32_t dpkg;
    uint32_t emerge;
    uint32_t eopkg;
    uint32_t flatpakSystem;
    uint32_t flatpakUser;
    uint32_t guixHome;
    uint32_t guixSystem;
    uint32_t guixUser;
    uint32_t hpkgSystem;
    uint32_t hpkgUser;
    uint32_t linglong;
    uint32_t lpkg;
    uint32_t lpkgbuild;
    uint32_t macports;
    uint32_t mport;
    uint32_t nixDefault;
    uint32_t nixSystem;
    uint32_t nixUser;
    uint32_t opkg;
    uint32_t pacman;
    uint32_t pacstall;
    uint32_t paludis;
    uint32_t pisi;
    uint32_t pkg;
    uint32_t pkgsrc;
    uint32_t pkgtool;
    uint32_t rpm;
    uint32_t scoopUser;
    uint32_t scoopGlobal;
    uint32_t snap;
    uint32_t soar;
    uint32_t sorcery;
    uint32_t winget;
    uint32_t xbps;

    uint32_t all; //Make sure this goes last

    FFstrbuf pacmanBranch;
} FFPackagesResult;

const char* ffDetectPackages(FFPackagesResult* result, FFPackagesOptions* options);
bool ffPackagesReadCache(FFstrbuf* cacheDir, FFstrbuf* cacheContent, const char* filePath, const char* packageId, uint32_t* result);
bool ffPackagesWriteCache(FFstrbuf* cacheDir, FFstrbuf* cacheContent, uint32_t num_elements);

#if defined(__linux__) || defined(__APPLE__) || defined(__GNU__)
uint32_t ffPackagesGetNix(FFstrbuf* baseDir, const char* dirname);
#endif
#ifndef _WIN32
uint32_t ffPackagesGetNumElements(const char* dirname, bool isdir);
#endif
