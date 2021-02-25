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

static void loadNoneLogo(FFlogo* logo)
{
    logo->width = 0;
    logo->height = 0;
    strcpy(logo->name, "none");
    strcpy(logo->color, "");
}

static void loadArchLogo(FFlogo* logo, bool doColor)
{
    logo->width = 37;
    logo->height = 20;
    strcpy(logo->name, "arch");

    const char* color = doColor ? "\033[36m" : "";
    strcpy(logo->color, "\033[36m");

    sprintf(logo->chars[0],  FASTFETCH_TEXT_MODIFIER_BOLT"%s                  -`                 "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[1],  FASTFETCH_TEXT_MODIFIER_BOLT"%s                  -`                 "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[2],  FASTFETCH_TEXT_MODIFIER_BOLT"%s                 .o+`                "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[3],  FASTFETCH_TEXT_MODIFIER_BOLT"%s                `ooo/                "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[4],  FASTFETCH_TEXT_MODIFIER_BOLT"%s               `+oooo:               "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[5],  FASTFETCH_TEXT_MODIFIER_BOLT"%s              `+oooooo:              "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[6],  FASTFETCH_TEXT_MODIFIER_BOLT"%s              -+oooooo+:             "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[7],  FASTFETCH_TEXT_MODIFIER_BOLT"%s            `/:-:++oooo+:            "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[8],  FASTFETCH_TEXT_MODIFIER_BOLT"%s           `/++++/+++++++:           "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[9],  FASTFETCH_TEXT_MODIFIER_BOLT"%s          `/++++++++++++++:          "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[10], FASTFETCH_TEXT_MODIFIER_BOLT"%s         `/+++ooooooooooooo/`        "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[11], FASTFETCH_TEXT_MODIFIER_BOLT"%s        ./ooosssso++osssssso+`       "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[12], FASTFETCH_TEXT_MODIFIER_BOLT"%s       .oossssso-````/ossssss+`      "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[13], FASTFETCH_TEXT_MODIFIER_BOLT"%s      -osssssso.      :ssssssso.     "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[14], FASTFETCH_TEXT_MODIFIER_BOLT"%s     :osssssss/        osssso+++.    "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[15], FASTFETCH_TEXT_MODIFIER_BOLT"%s    /ossssssss/        +ssssooo/-    "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[16], FASTFETCH_TEXT_MODIFIER_BOLT"%s  `/ossssso+/:-        -:/+osssso+-  "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[17], FASTFETCH_TEXT_MODIFIER_BOLT"%s `+sso+:-`                 `.-/+oso: "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[18], FASTFETCH_TEXT_MODIFIER_BOLT"%s`++:.                           `-/+/"FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[19], FASTFETCH_TEXT_MODIFIER_BOLT"%s.`                                 `/"FASTFETCH_TEXT_MODIFIER_RESET, color);
}

void ffLoadLogoSet(FFstate* state, const char* logo)
{
    if(strcasecmp(logo, "none") == 0)
    {
        loadNoneLogo(&state->logo);
        state->logo_seperator = 0; //This is wanted in most cases, so just set it
    }
    else if(strcasecmp(logo, "arch") == 0)
    {
        loadArchLogo(&state->logo, state->colorLogo);
    }
    else
    {
        if(state->showErrors)
            printf(FASTFETCH_TEXT_MODIFIER_ERROR"Error: unknown logo: %s"FASTFETCH_TEXT_MODIFIER_RESET"\n", logo);
        loadUnknownLogo(&state->logo);
    }
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
    for(int16_t i = 0; i < state->offsetx; i++)
        putchar(' ');

    int16_t cut = state->offsetx >= 0 ? 0 :
        (state->logo.width > state->offsetx * -1 ? state->offsetx * -1 : state->logo.width);

    if(state->current_row < state->logo.height)
    {
        printf(state->logo.chars[state->current_row] + cut);
    }
    else
    {
        for(uint8_t i = 0; i < state->logo.width - cut; i++)
            putchar(' ');   
    }
    
    for(uint16_t i = 0; i < state->logo_seperator; i++)
        putchar(' ');

    ++state->current_row;
}

static FFlogo* getLogos(uint8_t* size, bool color)
{
    #define FASTFETCH_LOGO_AMOUNT 3

    *size = FASTFETCH_LOGO_AMOUNT;
    static FFlogo logos[FASTFETCH_LOGO_AMOUNT];
    
    #undef FASTFETCH_LOGO_AMOUNT

    loadNoneLogo(&logos[0]);
    loadUnknownLogo(&logos[1]);
    loadArchLogo(&logos[2], color);

    return logos;
}

void ffListLogos()
{
    uint8_t size;
    FFlogo* logos = getLogos(&size, false);

    for(uint8_t i = 0; i < size; i++)
        puts(logos[i].name);
}

void ffPrintLogos(bool doColor)
{
    uint8_t size;
    FFlogo* logos = getLogos(&size, doColor);

    for(uint8_t i = 0; i < size; i++)
    {
        printf(FASTFETCH_TEXT_MODIFIER_BOLT"%s%s"FASTFETCH_TEXT_MODIFIER_RESET":\n", doColor ? logos[i].color : "", logos[i].name);
        for(uint8_t k = 0; k < logos[i].height; k++)
            puts(logos[i].chars[k]);
        putchar('\n');
    }
}