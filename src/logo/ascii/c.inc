#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo C[] = {
    #ifdef FASTFETCH_DATATEXT_LOGO_CACHYOS
    // CachyOS
    {
        .names = { "CachyOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_CACHYOS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CACHYOS_SMALL
    // CachyOSSmall
    {
        .names = { "CachyOS_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_CACHYOS_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CACHYOS_OLD_SMALL
    // CachyOSOldSmall
    {
        .names = { "CachyOS_old_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT | FF_LOGO_LINE_TYPE_ALTER_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_CACHYOS_OLD_SMALL,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CALCULATE
    // Calculate
    {
        .names = { "Calculate" },
        .lines = FASTFETCH_DATATEXT_LOGO_CALCULATE,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CALINIXOS
    // CalinixOS
    {
        .names = { "Calinix", "calinixos" },
        .lines = FASTFETCH_DATATEXT_LOGO_CALINIXOS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CALINIXOS_SMALL
    // CalinixOSSmall
    {
        .names = { "Calinix_small", "calinixos_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_CALINIXOS_SMALL,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CARBS
    // Carbs
    {
        .names = { "Carbs" },
        .lines = FASTFETCH_DATATEXT_LOGO_CARBS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CBL_MARINER
    // CBL-Mariner
    {
        .names = { "CBL-Mariner" },
        .lines = FASTFETCH_DATATEXT_LOGO_CBL_MARINER,
        .colors = {
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CELOS
    // CelOS
    {
        .names = { "Cel", "celos", "cel-linux", "celos-linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_CELOS,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_CYAN,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CENTER
    // Center
    {
        .names = { "Center" },
        .lines = FASTFETCH_DATATEXT_LOGO_CENTER,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CENTOS
    // CentOS
    {
        .names = { "CentOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_CENTOS,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CENTOS_SMALL
    // CentOSSmall
    {
        .names = { "CentOS_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_CENTOS_SMALL,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CEREUS
    // Cereus
    {
        .names = { "Cereus", "Cereus Linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_CEREUS,
        .colors = {
            FF_COLOR_FG_256 "173",
            FF_COLOR_FG_256 "108",
            FF_COLOR_FG_256 "71",
            FF_COLOR_FG_256 "151",
            FF_COLOR_FG_256 "72",
        },
        .colorKeys = FF_COLOR_FG_256 "108",
        .colorTitle = FF_COLOR_MODE_BOLD FF_COLOR_FG_WHITE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CHAKRA
    // Chakra
    {
        .names = { "Chakra" },
        .lines = FASTFETCH_DATATEXT_LOGO_CHAKRA,
        .colors = {
            FF_COLOR_FG_BLUE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CHALETOS
    // ChaletOS
    {
        .names = { "ChaletOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_CHALETOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CHAPEAU
    // Chapeau
    {
        .names = { "Chapeau" },
        .lines = FASTFETCH_DATATEXT_LOGO_CHAPEAU,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CHIMERA_LINUX
    // Chimera
    {
        .names = { "Chimera" },
        .lines = FASTFETCH_DATATEXT_LOGO_CHIMERA_LINUX,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CHIMERA_LINUX_SMALL
    // Chimera_small
    {
        .names = { "Chimera_small" },
        .lines = FASTFETCH_DATATEXT_LOGO_CHIMERA_LINUX_SMALL,
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CHONKYSEALOS
    // ChonkySealOS
    {
        .names = { "ChonkySealOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_CHONKYSEALOS,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CHROM
    // Chrom
    {
        .names = { "Chrom", "ChromeOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_CHROM,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_RED,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CLEANJARO
    // Cleanjaro
    {
        .names = { "Cleanjaro" },
        .lines = FASTFETCH_DATATEXT_LOGO_CLEANJARO,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CLEANJARO_SMALL
    // CleanjaroSmall
    {
        .names = { "Cleanjaro_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_CLEANJARO_SMALL,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_DEFAULT,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CLEAR_LINUX
    // ClearLinux
    {
        .names = { "Clear Linux", "clearlinux", "Clear Linux OS" },
        .lines = FASTFETCH_DATATEXT_LOGO_CLEAR_LINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CLEAROS
    // ClearOS
    {
        .names = { "ClearOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_CLEAROS,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_GREEN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CLOVER
    // Clover
    {
        .names = { "Clover" },
        .lines = FASTFETCH_DATATEXT_LOGO_CLOVER,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_COBALT
    // Cobalt
    {
        .names = { "Cobalt" },
        .lines = FASTFETCH_DATATEXT_LOGO_COBALT,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_LIGHT_BLACK,
            FF_COLOR_FG_LIGHT_BLUE,
            FF_COLOR_FG_BLACK,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CODEX
    // Codex Linux (reMarkable OS)
    {
        .names = { "Codex Linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_CODEX,
        .colors = {
            FF_COLOR_FG_WHITE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CONDRES
    // Condres
    {
        .names = { "Condres" },
        .lines = FASTFETCH_DATATEXT_LOGO_CONDRES,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_CYAN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_FEDORA_COREOS
    // ContainerLinux
    {
        .names = { "ContainerLinux", "Container Linux", "Container Linux by CoreOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_FEDORA_COREOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_TORIZONCORE
    // Common Torizon
    {
        .names = { "common-torizon" },
        .lines = FASTFETCH_DATATEXT_LOGO_TORIZONCORE,
        .colors = {
            FF_COLOR_FG_LIGHT_WHITE,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_BLUE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_COSMIC
    // Cosmic DE
    {
        .names = { "Cosmic" },
        .lines = FASTFETCH_DATATEXT_LOGO_COSMIC,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_LIGHT_YELLOW,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_LIGHT_RED,
            FF_COLOR_FG_RED,
        },
        .colorKeys = FF_COLOR_FG_LIGHT_RED,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CRUX
    // CRUX
    {
        .names = { "CRUX" },
        .lines = FASTFETCH_DATATEXT_LOGO_CRUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CRUX_SMALL
    // CRUXSmall
    {
        .names = { "CRUX_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_CRUX_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CRYSTAL
    // CrystalLinux
    {
        .names = { "Crystal", "Crystal", "crystal-linux", "Crystal-Linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_CRYSTAL,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CUCUMBER
    // Cucumber
    {
        .names = { "Cucumber", "CucumberOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_CUCUMBER,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CUERDOS
    // CuerdOS
    {
        .names = { "CuerdOS", "CuerdOS GNU/Linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_CUERDOS,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_GREEN,
        },
        .colorKeys = FF_COLOR_FG_GREEN,
        .colorTitle = FF_COLOR_FG_YELLOW,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CUTEFISHOS
    // CutefishOS
    {
        .names = { "CutefishOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_CUTEFISHOS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
        },
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CUTEOS
    // CuteOS
    {
        .names = { "CuteOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_CUTEOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_256 "57",
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CYBEROS
    // CyberOS
    {
        .names = { "CyberOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_CYBEROS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_256 "57",
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_CYAN,
    },
    #endif
    #ifdef FASTFETCH_DATATEXT_LOGO_CYCLEDREAM
    // cycledream
    {
        .names = { "cycledream" },
        .lines = FASTFETCH_DATATEXT_LOGO_CYCLEDREAM,
        .colors = {
            FF_COLOR_FG_MAGENTA,
        },
        .colorKeys = FF_COLOR_FG_MAGENTA,
        .colorTitle = FF_COLOR_FG_MAGENTA,
    },
    #endif
    // LAST
    {},
};
