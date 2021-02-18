#include "fastfetch.h"

#include <string.h>

static void loadArchLogo(FFstate* state)
{
    static const char* chars[] = {
        FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m                  -`                 "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m                 .o+`                "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m                `ooo/                "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m               `+oooo:               "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m              `+oooooo:              "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m              -+oooooo+:             "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m            `/:-:++oooo+:            "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m           `/++++/+++++++:           "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m          `/++++++++++++++:          "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m         `/+++ooooooooooooo/`        "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m        ./ooosssso++osssssso+`       "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m       .oossssso-````/ossssss+`      "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m      -osssssso.      :ssssssso.     "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m     :osssssss/        osssso+++.    "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m    /ossssssss/        +ssssooo/-    "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m  `/ossssso+/:-        -:/+osssso+-  "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m `+sso+:-`                 `.-/+oso: "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m`++:.                           `-/+/"FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m.`                                 `/"FASTFETCH_TEXT_MODIFIER_RESET
    };

    state->logo_width = 37;
    state->logo_height = 19;
    for(uint8_t i = 0; i < state->logo_height; i++)
        strcpy(state->logo_chars[i], chars[i]);
    strcpy(state->color, "\033[36m");
}

static void loadUnknownLogo(FFstate* state)
{
    static const char* chars[] = {
        FASTFETCH_TEXT_MODIFIER_BOLT"       ________        "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"   _jgN########Ngg_    "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT" _N##N@@\"\"  \"\"9NN##Np_ "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"d###P            N####p"FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"\"^^\"              T####"FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"                  d###P"FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"               _g###@F "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"            _gN##@P    "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"          gN###F\"      "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"         d###F         "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"        0###F          "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"        0###F          "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"        0###F          "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"        \"NN@'          "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"                       "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"         ___           "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"        q###r          "FASTFETCH_TEXT_MODIFIER_RESET,
        FASTFETCH_TEXT_MODIFIER_BOLT"         \"\"            "FASTFETCH_TEXT_MODIFIER_RESET
    };

    state->logo_width = 23;
    state->logo_height = 18;
    for(uint8_t i = 0; i < state->logo_height; i++)
        strcpy(state->logo_chars[i], chars[i]);
    state->color[0] = '\0';
}

void ffLoadLogoSet(FFstate* state, const char* logo)
{
    if(strcmp(logo, "arch") == 0)
        loadArchLogo(state);
    else
        loadUnknownLogo(state);
}

void ffLoadLogo(FFstate* state)
{
    char id[256];
    ffParsePropFile("/etc/os-release", "ID=%[^\n]", id);
    
    ffLoadLogoSet(state, id);
}

void ffPrintLogoLine(FFstate* state)
{
    if(state->current_row < state->logo_height)
    {
        printf(state->logo_chars[state->current_row]);
    }
    else
    {
        for(uint8_t i = 0; i < state->logo_width; i++)
            putchar(' ');   
    }
    
    for(uint8_t i = 0; i < state->logo_seperator; i++)
        putchar(' ');

    ++state->current_row;
}
