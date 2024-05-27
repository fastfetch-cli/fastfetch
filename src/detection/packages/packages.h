#pragma once

#include "fastfetch.h"

typedef struct FFPackagesResult
{
    uint32_t am;
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
    uint32_t lpkg;
    uint32_t lpkgbuild;
    uint32_t nixDefault;
    uint32_t nixSystem;
    uint32_t nixUser;
    uint32_t opkg;
    uint32_t pacman;
    uint32_t paludis;
    uint32_t pkg;
    uint32_t pkgtool;
    uint32_t macports;
    uint32_t rpm;
    uint32_t scoop;
    uint32_t snap;
    uint32_t sorcery;
    uint32_t winget;
    uint32_t xbps;

    uint32_t all; //Make sure this goes last

    FFstrbuf pacmanBranch;
} FFPackagesResult;

const char* ffDetectPackages(FFPackagesResult* result, FFPackagesOptions* options);
