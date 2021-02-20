#include "fastfetch.h"

#include <string.h>

static void loadUnknownLogo(FFlogo* logo)
{
    logo->width = 23;
    logo->height = 18;
    strcpy(logo->name, "unknown");
    strcpy(logo->color, "");
    strcpy(logo->chars[0], FASTFETCH_TEXT_MODIFIER_BOLT"       ________        "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[1], FASTFETCH_TEXT_MODIFIER_BOLT"   _jgN########Ngg_    "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[2], FASTFETCH_TEXT_MODIFIER_BOLT" _N##N@@\"\"  \"\"9NN##Np_ "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[3], FASTFETCH_TEXT_MODIFIER_BOLT"d###P            N####p"FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[4], FASTFETCH_TEXT_MODIFIER_BOLT"\"^^\"              T####"FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[5], FASTFETCH_TEXT_MODIFIER_BOLT"                  d###P"FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[6], FASTFETCH_TEXT_MODIFIER_BOLT"               _g###@F "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[7], FASTFETCH_TEXT_MODIFIER_BOLT"            _gN##@P    "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[8], FASTFETCH_TEXT_MODIFIER_BOLT"          gN###F\"      "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[9], FASTFETCH_TEXT_MODIFIER_BOLT"         d###F         "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[10], FASTFETCH_TEXT_MODIFIER_BOLT"        0###F          "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[11], FASTFETCH_TEXT_MODIFIER_BOLT"        0###F          "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[12], FASTFETCH_TEXT_MODIFIER_BOLT"        0###F          "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[13], FASTFETCH_TEXT_MODIFIER_BOLT"        \"NN@'          "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[14], FASTFETCH_TEXT_MODIFIER_BOLT"                       "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[15], FASTFETCH_TEXT_MODIFIER_BOLT"         ___           "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[16], FASTFETCH_TEXT_MODIFIER_BOLT"        q###r          "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[17], FASTFETCH_TEXT_MODIFIER_BOLT"         \"\"            "FASTFETCH_TEXT_MODIFIER_RESET);
}

static void loadArchLogo(FFlogo* logo)
{
    logo->width = 37;
    logo->height = 20;
    strcpy(logo->name, "arch");
    strcpy(logo->color, "\033[36m");
    strcpy(logo->chars[0],  FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m                  -`                 "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[1],  FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m                  -`                 "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[2],  FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m                 .o+`                "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[3],  FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m                `ooo/                "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[4],  FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m               `+oooo:               "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[5],  FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m              `+oooooo:              "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[6],  FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m              -+oooooo+:             "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[7],  FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m            `/:-:++oooo+:            "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[8],  FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m           `/++++/+++++++:           "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[9],  FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m          `/++++++++++++++:          "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[10], FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m         `/+++ooooooooooooo/`        "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[11], FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m        ./ooosssso++osssssso+`       "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[12], FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m       .oossssso-````/ossssss+`      "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[13], FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m      -osssssso.      :ssssssso.     "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[14], FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m     :osssssss/        osssso+++.    "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[15], FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m    /ossssssss/        +ssssooo/-    "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[16], FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m  `/ossssso+/:-        -:/+osssso+-  "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[17], FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m `+sso+:-`                 `.-/+oso: "FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[18], FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m`++:.                           `-/+/"FASTFETCH_TEXT_MODIFIER_RESET);
    strcpy(logo->chars[19], FASTFETCH_TEXT_MODIFIER_BOLT"\033[36m.`                                 `/"FASTFETCH_TEXT_MODIFIER_RESET);
}

static FFlogo* getLogos(uint8_t* size)
{
    #define FASTFETCH_LOGO_AMOUNT 2

    *size = FASTFETCH_LOGO_AMOUNT;
    static FFlogo logos[FASTFETCH_LOGO_AMOUNT];
    
    #undef FASTFETCH_LOGO_AMOUNT

    loadUnknownLogo(&logos[0]);
    loadArchLogo(&logos[1]);

    return logos;
}

void ffLoadLogoSet(FFstate* state, const char* logo)
{
    if(strcmp(logo, "arch") == 0)
    {
        loadArchLogo(&state->logo);
    }
    else
    {
        if(state->showErrors)
            printf(FASTFETCH_TEXT_MODIFIER_ERROR"Error: unknown logo: %s"FASTFETCH_TEXT_MODIFIER_RESET"\n", logo);
        loadUnknownLogo(&state->logo);
    }

    strcpy(state->color, state->logo.color);
}

void ffLoadLogo(FFstate* state)
{
    char id[256];
    ffParsePropFile("/etc/os-release", "ID=%[^\n]", id);
    if(id[0] == '\0')
    {
        if(state->showErrors)
            puts(FASTFETCH_TEXT_MODIFIER_ERROR"Error: \"ID=%[^\\n]\" not found in \"/etc/os-release\""FASTFETCH_TEXT_MODIFIER_RESET);
        loadUnknownLogo(&state->logo);
    }
    else
    {
        ffLoadLogoSet(state, id);
    }
}

void ffPrintLogoLine(FFstate* state)
{
    if(state->current_row < state->logo.height)
    {
        printf(state->logo.chars[state->current_row]);
    }
    else
    {
        for(uint8_t i = 0; i < state->logo.width; i++)
            putchar(' ');   
    }
    
    for(uint16_t i = 0; i < state->logo_seperator; i++)
        putchar(' ');

    ++state->current_row;
}

void ffListLogos()
{
    uint8_t size;
    FFlogo* logos = getLogos(&size);

    for(uint8_t i = 0; i < size; i++)
        puts(logos[i].name);
}

void ffPrintLogos()
{
    uint8_t size;
    FFlogo* logos = getLogos(&size);

    for(uint8_t i = 0; i < size; i++)
    {
        printf(FASTFETCH_TEXT_MODIFIER_BOLT"%s%s"FASTFETCH_TEXT_MODIFIER_RESET":\n", logos[i].color, logos[i].name);
        for(uint8_t k = 0; k < logos[i].height; k++)
            puts(logos[i].chars[k]);
        putchar('\n');
    }
}