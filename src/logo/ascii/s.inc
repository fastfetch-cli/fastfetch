#include "../logo.h"
#include "logo_builtin.h"
#include "common/color.h"

static const FFlogo S[] = {
    // Sabayon
    {
        .names = { "Sabayon" },
        .lines = FASTFETCH_DATATEXT_LOGO_SABAYON,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // Sabotage
    {
        .names = { "Sabotage" },
        .lines = FASTFETCH_DATATEXT_LOGO_SABOTAGE,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Sailfish
    {
        .names = { "Sailfish" },
        .lines = FASTFETCH_DATATEXT_LOGO_SAILFISH,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
        },
    },
    // SalentOS
    {
        .names = { "SalentOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_SALENTOS,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_RED,
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_WHITE,
        },
    },
    // SalientOS
    {
        .names = { "Salient OS", "SalientOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_SALIENTOS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // Salix
    {
        .names = { "Salix" },
        .lines = FASTFETCH_DATATEXT_LOGO_SALIX,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_GREEN,
        },
    },
    // SambaBOX
    {
        .names = { "SambaBOX", "Profelis SambaBOX" },
        .lines = FASTFETCH_DATATEXT_LOGO_SAMBABOX,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_CYAN,
        },
    },
    // Sasanqua
    {
        .names = { "Sasanqua" },
        .lines = FASTFETCH_DATATEXT_LOGO_SASANQUA,
        .colors = {
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_RED,
        },
    },
    // Scientific
    {
        .names = { "Scientific" },
        .lines = FASTFETCH_DATATEXT_LOGO_SCIENTIFIC,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_RED,
        },
    },
    // Secureblue
    {
        .names = { "secureblue" },
        .lines = FASTFETCH_DATATEXT_LOGO_SECUREBLUE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_MAGENTA,
            FF_COLOR_FG_DEFAULT,
        },
    },
    // Serpent OS
    {
        .names = { "Serpent OS" },
        .lines = FASTFETCH_DATATEXT_LOGO_SERPENT_OS,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
    },
    // Semc
    {
        .names = { "semc" },
        .lines = FASTFETCH_DATATEXT_LOGO_SEMC,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_LIGHT_BLACK,
            FF_COLOR_FG_RED,
        },
    },
    // Septor
    {
        .names = { "Septor" },
        .lines = FASTFETCH_DATATEXT_LOGO_SEPTOR,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_BLUE,
        },
    },
    // Serene
    {
        .names = { "Serene" },
        .lines = FASTFETCH_DATATEXT_LOGO_SERENE,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_CYAN,
        },
    },
    // SharkLinux
    {
        .names = { "SharkLinux" },
        .lines = FASTFETCH_DATATEXT_LOGO_SHARKLINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // ShastraOS
    {
        .names = { "ShastraOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_SHASTRAOS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // Shebang
    {
        .names = { "Shebang" },
        .lines = FASTFETCH_DATATEXT_LOGO_SHEBANG,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_WHITE,
        },
    },
    // Siduction
    {
        .names = { "Siduction" },
        .lines = FASTFETCH_DATATEXT_LOGO_SIDUCTION,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // SkiffOS
    {
        .names = { "SkiffOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_SKIFFOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // SleeperOS
    {
        .names = { "SleeperOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_SLEEPEROS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // SleeperOSSmall
    {
        .names = { "SleeperOS_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_SLEEPEROS_SMALL,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // Slitaz
    {
        .names = { "Slitaz" },
        .lines = FASTFETCH_DATATEXT_LOGO_SLITAZ,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_YELLOW,
        },
    },
    // SpoinkOS
    {
        .names = { "SpoinkOS", "spoink-os" },
        .lines = FASTFETCH_DATATEXT_LOGO_SPOINKOS,
        .colors = {
            FF_COLOR_FG_GREEN,
        },
    },
    // Slackel
    {
        .names = { "Slackel" },
        .lines = FASTFETCH_DATATEXT_LOGO_SLACKEL,
        .colors = {
            FF_COLOR_FG_YELLOW,
            FF_COLOR_FG_YELLOW,
        },
    },
    // Slackware
    {
        .names = { "Slackware" },
        .lines = FASTFETCH_DATATEXT_LOGO_SLACKWARE,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // SlackwareSmall
    {
        .names = { "Slackware_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_SLACKWARE_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // SmartOS
    {
        .names = { "SmartOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_SMARTOS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // SnigdhaOS
    {
        .names = { "SnigdhaOS", "Snigdha" },
        .lines = FASTFETCH_DATATEXT_LOGO_SNIGDHAOS,
        .colors = {
            FF_COLOR_FG_CYAN,
            FF_COLOR_FG_WHITE,
        },
    },
    // Soda
    {
        .names = { "Soda" },
        .lines = FASTFETCH_DATATEXT_LOGO_SODA,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // Source Mage
    {
        .names = { "Source Mage", "Source Mage GNU/Linux", "source_mage", "sourcemage" },
        .lines = FASTFETCH_DATATEXT_LOGO_SOURCE_MAGE,
        .colors = {
            FF_COLOR_FG_DEFAULT,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Solaris
    {
        .names = { "solaris", "sunos" },
        .lines = FASTFETCH_DATATEXT_LOGO_SOLARIS,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // SolarisSmall
    {
        .names = { "solaris_small", "sunos_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_SOLARIS_SMALL,
        .colors = {
            FF_COLOR_FG_YELLOW,
        },
        .colorKeys = FF_COLOR_FG_YELLOW,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Solus
    {
        .names = { "Solus", "solus-linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_SOLUS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_DEFAULT,
    },
    // Sparky
    {
        .names = { "Sparky" },
        .lines = FASTFETCH_DATATEXT_LOGO_SPARKY,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
    },
    // Star
    {
        .names = { "Star" },
        .lines = FASTFETCH_DATATEXT_LOGO_STAR,
        .colors = {
            FF_COLOR_FG_WHITE,
            FF_COLOR_FG_WHITE,
        },
    },
    // StockLinux
    {
        .names = { "Stock Linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_STOCK_LINUX,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // SteamOS
    {
        .names = { "SteamOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_STEAMOS,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Steam Deck
    {
        .names = { "SteamDeck" },
        .lines = FASTFETCH_DATATEXT_LOGO_STEAMDECK,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Steam Deck Small
    {
        .names = { "SteamDeck_small" },
        .lines = FASTFETCH_DATATEXT_LOGO_STEAMDECK_SMALL,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_BLUE,
        .colorTitle = FF_COLOR_FG_BLUE,
    },
    // Steam Deck OLED
    {
        .names = { "SteamDeckOled" },
        .lines = FASTFETCH_DATATEXT_LOGO_STEAMDECK,
        .colors = {
            FF_COLOR_FG_RED,
            FF_COLOR_FG_WHITE,
        },
        .colorKeys = FF_COLOR_FG_RED,
        .colorTitle = FF_COLOR_FG_RED,
    },
    // Sulin
    {
        .names = { "Sulin" },
        .lines = FASTFETCH_DATATEXT_LOGO_SULIN,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // SummitOS
    {
        .names = { "SummitOS" },
        .lines = FASTFETCH_DATATEXT_LOGO_SUMMITOS,
        .colors = {
            FF_COLOR_FG_RGB "143;191;80",
            FF_COLOR_FG_RGB "160;205;102",
            FF_COLOR_FG_RGB "181;225;102",
        },
    },
    // Suse
    {
        .names = { "suse", "suse-linux" },
        .lines = FASTFETCH_DATATEXT_LOGO_SUSE,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_GREEN,
        },
    },
    // SuseSmall
    {
        .names = { "suse_small", "suse-linux_small" },
        .type = FF_LOGO_LINE_TYPE_SMALL_BIT,
        .lines = FASTFETCH_DATATEXT_LOGO_OPENSUSE_SMALL,
        .colors = {
            FF_COLOR_FG_GREEN,
            FF_COLOR_FG_GREEN,
        },
    },
    // Swagarch
    {
        .names = { "Swagarch" },
        .lines = FASTFETCH_DATATEXT_LOGO_SWAGARCH,
        .colors = {
            FF_COLOR_FG_BLUE,
            FF_COLOR_FG_WHITE,
        },
    },
    // LAST
    {},
};
