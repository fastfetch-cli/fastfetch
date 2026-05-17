#include "board.h"
#include "common/settings.h"

//#include <cflib.h>
#include <mint/cookie.h>

# define COOKIE__MCH /*(const char *)*/0x5f4d4348L
// TODO: COOKIE_HADES

const char* ffDetectBoard(FFBoardResult* board) {
    long value;
    const char *vendor = "Unknown";
    const char *product = "Unknown";

    // cf. https://www.exxosforum.co.uk/atari/mirror/toshyp/003007.html#Cookie_2C_20_MCH
    static struct machine_cookies {
        long v;
        const char *vendor;
        const char *product;
    } machine_cookies[] = {
        { 0x00000000, "Atari", "ST" }, // (260 ST,520 ST,1040 ST,Mega ST,...)
        { 0x00004D34, "Medusa", "T40 without SCSI" },
        { 0x00010000, "Atari", "STE" }, // (1040 STE, ST-Book)
        { 0x00010001, "Atari", "ST-Book" }, // (1040 STE, ST-Book)
        { 0x00010008, "Atari", "STE with IDE" },
        { 0x00010010, "Atari", "Mega STE" },
        { 0x00010008, "Atari", "Mega STE with IDE" },
        { 0x00010100, "Atari", "Sparrow" }, // (Falcon pre-production machine)
        { 0x00020000, "Atari", "TT or Hades" },
        { 0x00024D34, "Medusa", "T40 with SCSI" },
        { 0x00030000, "Atari", "Falcon 030" },
        { 0x00040000, "Milan", "Milan" },
        { 0x00050000, "ARAnyM", "ARAnyM"}, // >=v0.8.5beta
        // ACP / FireBee?
        // Vampire?
        { -1, "Unknown", "Non ST compatible" },
        { -1, NULL, NULL }
    };

    if (Getcookie(COOKIE__MCH, &value)) {
        return "getcookie(_MCH) failed";
    }
    for (int i = 0; machine_cookies[i].vendor; i++)
        if (machine_cookies[i].v == value) {
            vendor = machine_cookies[i].vendor;
            product = machine_cookies[i].product;
            break;
        }

    ffStrbufSetStatic(&board->vendor, vendor);
    ffStrbufSetStatic(&board->name, product);
    //ffStrbufSetStatic(&board->version, ""));
    //ffStrbufSetStatic(&board->serial, "");

    return NULL;
}
