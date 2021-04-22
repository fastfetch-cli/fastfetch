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

static void loadArtixLogo(FFlogo* logo, bool doColor)
{
    logo->width = 38;
    logo->height = 20;
    strcpy(logo->name, "artix");

    const char* color = doColor ? "\033[36m" : "";
    strcpy(logo->color, "\033[36m");

    sprintf(logo->chars[0],  FASTFETCH_TEXT_MODIFIER_BOLT"%s                   '                  "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[1],  FASTFETCH_TEXT_MODIFIER_BOLT"%s                  'o'                 "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[2],  FASTFETCH_TEXT_MODIFIER_BOLT"%s                 'ooo'                "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[3],  FASTFETCH_TEXT_MODIFIER_BOLT"%s                'ooxoo'               "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[4],  FASTFETCH_TEXT_MODIFIER_BOLT"%s               'ooxxxoo'              "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[5],  FASTFETCH_TEXT_MODIFIER_BOLT"%s              'oookkxxoo'             "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[6],  FASTFETCH_TEXT_MODIFIER_BOLT"%s             'oiioxkkxxoo'            "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[7],  FASTFETCH_TEXT_MODIFIER_BOLT"%s            ':;:iiiioxxxoo'           "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[8],  FASTFETCH_TEXT_MODIFIER_BOLT"%s              `'.;::ioxxoo''          "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[9],  FASTFETCH_TEXT_MODIFIER_BOLT"%s          '-.      `':;jiooo'         "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[10], FASTFETCH_TEXT_MODIFIER_BOLT"%s        'oooio-..     `'i:io'         "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[11], FASTFETCH_TEXT_MODIFIER_BOLT"%s       'ooooxxxxoio:,.   `'-;'        "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[12], FASTFETCH_TEXT_MODIFIER_BOLT"%s      'ooooxxxxxkkxoooIi:-.  `'       "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[13], FASTFETCH_TEXT_MODIFIER_BOLT"%s     'ooooxxxxxkkkkxoiiiiiji'         "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[14], FASTFETCH_TEXT_MODIFIER_BOLT"%s    'ooooxxxxxkxxoiiii:'`     .i'     "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[15], FASTFETCH_TEXT_MODIFIER_BOLT"%s   'ooooxxxxxoi:::'`       .;ioxo'    "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[16], FASTFETCH_TEXT_MODIFIER_BOLT"%s  'ooooxooi::'`         .:iiixkxxo'   "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[17], FASTFETCH_TEXT_MODIFIER_BOLT"%s 'ooooi:'`                `'';ioxxo'  "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[18], FASTFETCH_TEXT_MODIFIER_BOLT"%s'i:'`                          '':io' "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[19], FASTFETCH_TEXT_MODIFIER_BOLT"%s'`                                  `'"FASTFETCH_TEXT_MODIFIER_RESET, color);
}

static void loadUbuntuLogo(FFlogo *logo, bool doColor)
{
    logo->width = 40;
    logo->height = 20;
    strcpy(logo->name, "ubuntu");

    /* color = red; color2 = white */
    const char* color = doColor ? "\033[91m" : "";
    const char* color2 = doColor ? "\033[97m" : "";
    strcpy(logo->color, "\033[91m");

    sprintf(logo->chars[0], FASTFETCH_TEXT_MODIFIER_BOLT"%s             .-/+oossssoo+/-.           "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[1], FASTFETCH_TEXT_MODIFIER_BOLT"%s         `:+ssssssssssssssssss+:`       "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[2], FASTFETCH_TEXT_MODIFIER_BOLT"%s       -+ssssssssssssssssssyyssss+-     "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[3], FASTFETCH_TEXT_MODIFIER_BOLT"%s     .ossssssssssssssssssd%sMMMNy%ssssso.   "FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color);
    sprintf(logo->chars[4], FASTFETCH_TEXT_MODIFIER_BOLT"%s   /sssssssssss%shdmmNNmmyNMMMMh%sssssss/   "FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color);
    sprintf(logo->chars[5], FASTFETCH_TEXT_MODIFIER_BOLT"%s  +sssssssss%shmydMMMMMMMNddddy%sssssssss+  "FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color);
    sprintf(logo->chars[6], FASTFETCH_TEXT_MODIFIER_BOLT"%s /ssssssss%shNMMMyhhyyyyhmNMMMNh%sssssssss/ "FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color);
    sprintf(logo->chars[7], FASTFETCH_TEXT_MODIFIER_BOLT"%s.ssssssss%sdMMMNh%sssssssssss%shNMMMd%sssssssss."FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color, color2, color);
    sprintf(logo->chars[8], FASTFETCH_TEXT_MODIFIER_BOLT"%s+ssss%shhhyNMMNy%sssssssssssss%syNMMMy%ssssssss+"FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color, color2, color);
    sprintf(logo->chars[9], FASTFETCH_TEXT_MODIFIER_BOLT"%soss%syNMMMNyMMh%sssssssssssssss%shmmmh%sssssssso"FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color, color2, color);
    sprintf(logo->chars[10], FASTFETCH_TEXT_MODIFIER_BOLT"%soss%syNMMMNyMMh%sssssssssssssss%shmmmh%sssssssso"FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color, color2, color);
    sprintf(logo->chars[11], FASTFETCH_TEXT_MODIFIER_BOLT"%s+ssss%shhhyNMMNy%sssssssssssss%syNMMMy%ssssssss+"FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color, color2, color);
    sprintf(logo->chars[12], FASTFETCH_TEXT_MODIFIER_BOLT"%s.ssssssss%sdMMMNh%sssssssssss%shNMMMd%sssssssss."FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color, color2, color);
    sprintf(logo->chars[13], FASTFETCH_TEXT_MODIFIER_BOLT"%s /ssssssss%shNMMMyhhyyyyhdNMMMNh%sssssssss/ "FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color);
    sprintf(logo->chars[14], FASTFETCH_TEXT_MODIFIER_BOLT"%s  +sssssssss%sdmydMMMMMMMMddddy%sssssssss+  "FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color);
    sprintf(logo->chars[15], FASTFETCH_TEXT_MODIFIER_BOLT"%s   /sssssssssss%shdmNNNNmyNMMMMh%sssssss/   "FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color);
    sprintf(logo->chars[16], FASTFETCH_TEXT_MODIFIER_BOLT"%s    .ossssssssssssssssss%sdMMMNy%ssssso.    "FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color);
    sprintf(logo->chars[17], FASTFETCH_TEXT_MODIFIER_BOLT"%s     -+sssssssssssssssss%syyy%sssss+-       "FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color);
    sprintf(logo->chars[18], FASTFETCH_TEXT_MODIFIER_BOLT"%s       `:+ssssssssssssssssss+:`         "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[19], FASTFETCH_TEXT_MODIFIER_BOLT"%s           .-/+oossssoo+/-.             "FASTFETCH_TEXT_MODIFIER_RESET, color);
}

static void loadDebianLogo(FFlogo *logo, bool doColor)
{
    logo->width = 27;
    logo->height = 17;
    strcpy(logo->name, "debian");

    /* color = red; color2 = white */
    const char* color = doColor ? "\033[91m" : "";
    const char* color2 = doColor ? "\033[97m" : "";
    strcpy(logo->color, "\033[91m");

    sprintf(logo->chars[0], FASTFETCH_TEXT_MODIFIER_BOLT"%s       _,met$$$$$gg.       "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[1], FASTFETCH_TEXT_MODIFIER_BOLT"%s    ,g$$$$$$$$$$$$$$$P.    "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[2], FASTFETCH_TEXT_MODIFIER_BOLT"%s  ,g$$P\"     \"\"\"Y$$.\".     "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[3], FASTFETCH_TEXT_MODIFIER_BOLT"%s ,$$P'              `$$$.  "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[4], FASTFETCH_TEXT_MODIFIER_BOLT"%s',$$P       ,ggs.     `$$b:"FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[5], FASTFETCH_TEXT_MODIFIER_BOLT"%s`d$$'     ,$P\"'   %s.%s    $$$ "FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color);
    sprintf(logo->chars[6], FASTFETCH_TEXT_MODIFIER_BOLT"%s $$P      d$'     %s,%s    $$P "FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color);
    sprintf(logo->chars[7], FASTFETCH_TEXT_MODIFIER_BOLT"%s $$:      $$.   %s-%s    ,d$$' "FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color);
    sprintf(logo->chars[8], FASTFETCH_TEXT_MODIFIER_BOLT"%s $$;      Y$b._   _,d$P'   "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[9], FASTFETCH_TEXT_MODIFIER_BOLT"%s Y$$.    %s`.%s`\"Y$$$$P\"'      "FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color);
    sprintf(logo->chars[10], FASTFETCH_TEXT_MODIFIER_BOLT"%s `$$b      %s\"-.__%s           "FASTFETCH_TEXT_MODIFIER_RESET, color, color2, color);
    sprintf(logo->chars[11], FASTFETCH_TEXT_MODIFIER_BOLT"%s  `Y$$                     "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[12], FASTFETCH_TEXT_MODIFIER_BOLT"%s   `Y$$.                   "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[13], FASTFETCH_TEXT_MODIFIER_BOLT"%s     `$$b.                 "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[14], FASTFETCH_TEXT_MODIFIER_BOLT"%s       `Y$$b.              "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[15], FASTFETCH_TEXT_MODIFIER_BOLT"%s          `\"Y$b._          "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[16], FASTFETCH_TEXT_MODIFIER_BOLT"%s             `\"\"\"          "FASTFETCH_TEXT_MODIFIER_RESET, color);
}

static void loadManjaroLogo(FFlogo *logo, bool doColor)
{
    logo->width = 28;
    logo->height = 14;
    strcpy(logo->name, "manjaro");

    /* color = light green */
    const char* color = doColor ? "\033[92m" : "";
    strcpy(logo->color, "\033[92m");

	sprintf(logo->chars[0],  FASTFETCH_TEXT_MODIFIER_BOLT"%s██████████████████  ████████"FASTFETCH_TEXT_MODIFIER_RESET, color);
	sprintf(logo->chars[1],  FASTFETCH_TEXT_MODIFIER_BOLT"%s██████████████████  ████████"FASTFETCH_TEXT_MODIFIER_RESET, color);
	sprintf(logo->chars[2],  FASTFETCH_TEXT_MODIFIER_BOLT"%s██████████████████  ████████"FASTFETCH_TEXT_MODIFIER_RESET, color);
	sprintf(logo->chars[3],  FASTFETCH_TEXT_MODIFIER_BOLT"%s██████████████████  ████████"FASTFETCH_TEXT_MODIFIER_RESET, color);
	sprintf(logo->chars[4],  FASTFETCH_TEXT_MODIFIER_BOLT"%s████████            ████████"FASTFETCH_TEXT_MODIFIER_RESET, color);
	sprintf(logo->chars[5],  FASTFETCH_TEXT_MODIFIER_BOLT"%s████████  ████████  ████████"FASTFETCH_TEXT_MODIFIER_RESET, color);
	sprintf(logo->chars[6],  FASTFETCH_TEXT_MODIFIER_BOLT"%s████████  ████████  ████████"FASTFETCH_TEXT_MODIFIER_RESET, color);
	sprintf(logo->chars[7],  FASTFETCH_TEXT_MODIFIER_BOLT"%s████████  ████████  ████████"FASTFETCH_TEXT_MODIFIER_RESET, color);
	sprintf(logo->chars[8],  FASTFETCH_TEXT_MODIFIER_BOLT"%s████████  ████████  ████████"FASTFETCH_TEXT_MODIFIER_RESET, color);
	sprintf(logo->chars[9],  FASTFETCH_TEXT_MODIFIER_BOLT"%s████████  ████████  ████████"FASTFETCH_TEXT_MODIFIER_RESET, color);
	sprintf(logo->chars[10], FASTFETCH_TEXT_MODIFIER_BOLT"%s████████  ████████  ████████"FASTFETCH_TEXT_MODIFIER_RESET, color);
	sprintf(logo->chars[11], FASTFETCH_TEXT_MODIFIER_BOLT"%s████████  ████████  ████████"FASTFETCH_TEXT_MODIFIER_RESET, color);
	sprintf(logo->chars[12], FASTFETCH_TEXT_MODIFIER_BOLT"%s████████  ████████  ████████"FASTFETCH_TEXT_MODIFIER_RESET, color);
	sprintf(logo->chars[13], FASTFETCH_TEXT_MODIFIER_BOLT"%s████████  ████████  ████████"FASTFETCH_TEXT_MODIFIER_RESET, color);
}

static void loadVoidLogo(FFlogo *logo, bool doColor)
{
    logo->width = 45;
    logo->height = 18;
    strcpy(logo->name, "void");

    /* color = light green; color2 = gray */
    const char* color = doColor ? "\033[92m" : "";
    const char* color2 = doColor ? "\033[30m" : "";
    strcpy(logo->color, "\033[92m");

    sprintf(logo->chars[0],  FASTFETCH_TEXT_MODIFIER_BOLT"%s                __.;=====;.__                "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[1],  FASTFETCH_TEXT_MODIFIER_BOLT"%s            _.=+==++=++=+=+===;.             "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[2],  FASTFETCH_TEXT_MODIFIER_BOLT"%s             -=+++=+===+=+=+++++=_           "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[3],  FASTFETCH_TEXT_MODIFIER_BOLT"%s        .     -=:``     `--==+=++==.         "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[4],  FASTFETCH_TEXT_MODIFIER_BOLT"%s       _vi,    `            --+=++++:        "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[5],  FASTFETCH_TEXT_MODIFIER_BOLT"%s      .uvnvi.       _._       -==+==+.       "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[6],  FASTFETCH_TEXT_MODIFIER_BOLT"%s     .vvnvnI`    .;==|==;.     :|=||=|.      "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[7],  FASTFETCH_TEXT_MODIFIER_BOLT"%s+QmQQm%spvvnv;%s _yYsyQQWUUQQQm #QmQ#%s:%sQQQWUV$QQm."FASTFETCH_TEXT_MODIFIER_RESET, color2, color, color2, color, color2);
    sprintf(logo->chars[8],  FASTFETCH_TEXT_MODIFIER_BOLT"%s -QQWQW%spvvo%swZ?.wQQQE%s==<%sQWWQ/QWQW.QQWW%s(:%s jQWQE"FASTFETCH_TEXT_MODIFIER_RESET, color2, color, color2, color, color2, color, color2);
    sprintf(logo->chars[9],  FASTFETCH_TEXT_MODIFIER_BOLT"%s  -$QQQQmmU'  jQQQ%s@+=<%sQWQQ)mQQQ.mQQQC%s+;%sjWQQ@'"FASTFETCH_TEXT_MODIFIER_RESET, color2, color, color2, color, color2);
    sprintf(logo->chars[10], FASTFETCH_TEXT_MODIFIER_BOLT"%s   -$WQ8Y%snI:%s   QWQQwgQQWV%s`%smWQQ.jQWQQgyyWW@!  "FASTFETCH_TEXT_MODIFIER_RESET, color2, color, color2, color, color2);
    sprintf(logo->chars[11], FASTFETCH_TEXT_MODIFIER_BOLT"%s     -1vvnvv.     `~+++`        ++|+++       "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[12], FASTFETCH_TEXT_MODIFIER_BOLT"%s      +vnvnnv,                 `-|===        "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[13], FASTFETCH_TEXT_MODIFIER_BOLT"%s       +vnvnvns.           .      :=-        "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[14], FASTFETCH_TEXT_MODIFIER_BOLT"%s        -Invnvvnsi..___..=sv=.     `         "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[15], FASTFETCH_TEXT_MODIFIER_BOLT"%s          +Invnvnvnnnnnnnnvvnn;.             "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[16], FASTFETCH_TEXT_MODIFIER_BOLT"%s            ~|Invnvnvvnvvvnnv}+`             "FASTFETCH_TEXT_MODIFIER_RESET, color);
    sprintf(logo->chars[17], FASTFETCH_TEXT_MODIFIER_BOLT"%s               -~|{*l}*|~                    "FASTFETCH_TEXT_MODIFIER_RESET, color);
}

void ffLoadLogoSet(FFconfig* config, const char* logo)
{
    if(strcasecmp(logo, "none") == 0)
    {
        loadNoneLogo(&config->logo);
        config->logo_spacing = 0; //This is wanted in most cases, so just set it
    }
    else if(strcasecmp(logo, "arch") == 0)
        loadArchLogo(&config->logo, config->colorLogo);
    else if(strcasecmp(logo, "artix") == 0)
        loadArtixLogo(&config->logo, config->colorLogo);
    else if(strcasecmp(logo, "ubuntu") == 0)
        loadUbuntuLogo(&config->logo, config->colorLogo);
    else if(strcasecmp(logo, "debian") == 0)
        loadDebianLogo(&config->logo, config->colorLogo);
    else if(strcasecmp(logo, "manjaro") == 0)
        loadManjaroLogo(&config->logo, config->colorLogo);
    else if(strcasecmp(logo, "void") == 0)
        loadVoidLogo(&config->logo, config->colorLogo);
    else if(strcasecmp(logo, "unknown") == 0)
        loadUnknownLogo(&config->logo);
    else
    {
        if(config->showErrors)
            printf(FASTFETCH_TEXT_MODIFIER_ERROR"Error: unknown logo: %s"FASTFETCH_TEXT_MODIFIER_RESET"\n", logo);
        loadUnknownLogo(&config->logo);
    }
}

void ffLoadLogo(FFinstance* instance)
{
    FFstrbuf cacheFilePath;
    ffStrbufInitA(&cacheFilePath, 64);
    ffGetCacheFilePath(instance, "logo.ffcl", NULL, &cacheFilePath);

    if(!instance->config.recache)
    {
        FFstrbuf content;
        ffStrbufInit(&content);
        ffAppendFileContent(cacheFilePath.chars, &content);

        if(content.length > 0)
        {
            ffLoadLogoSet(&instance->config, content.chars);
            ffStrbufDestroy(&content);
            ffStrbufDestroy(&cacheFilePath);
            return;
        }

        ffStrbufDestroy(&content);
    }

    const FFOSResult* result = ffCalculateOS(instance);

    if(result->id.length > 0)
    {
        ffLoadLogoSet(&instance->config, result->id.chars);
        ffWriteFileContent(cacheFilePath.chars, &result->id);
    }
    else if(result->name.length > 0)
    {
        ffLoadLogoSet(&instance->config, result->name.chars);
        ffWriteFileContent(cacheFilePath.chars, &result->name);
    }
    else
        loadUnknownLogo(&instance->config.logo);

    ffStrbufDestroy(&cacheFilePath);
}

void ffPrintLogoLine(FFinstance* instance)
{
    for(int16_t i = 0; i < instance->config.offsetx; i++)
        putchar(' ');

    int16_t cut = instance->config.offsetx >= 0 ? 0 :
        (instance->config.logo.width > instance->config.offsetx * -1 ? instance->config.offsetx * -1 : instance->config.logo.width);

    if(instance->state.current_row < instance->config.logo.height)
    {
        printf("%s", instance->config.logo.chars[instance->state.current_row] + cut);
    }
    else
    {
        for(uint8_t i = 0; i < instance->config.logo.width - cut; i++)
            putchar(' ');
    }

    for(uint16_t i = 0; i < instance->config.logo_spacing; i++)
        putchar(' ');

    ++instance->state.current_row;
}

void ffPrintRemainingLogo(FFinstance* instance)
{
    while(instance->state.current_row < instance->config.logo.height)
    {
        ffPrintLogoLine(instance);
        putchar('\n');
    }
}

#ifndef FASTFETCH_BUILD_FLASHFETCH

static FFlogo* getLogos(uint8_t* size, bool color)
{
    #define FASTFETCH_LOGO_AMOUNT 8

    *size = FASTFETCH_LOGO_AMOUNT;
    static FFlogo logos[FASTFETCH_LOGO_AMOUNT];

    #undef FASTFETCH_LOGO_AMOUNT

    loadNoneLogo(&logos[0]);
    loadUnknownLogo(&logos[1]);
    loadArchLogo(&logos[2], color);
    loadArtixLogo(&logos[3], color);
    loadUbuntuLogo(&logos[4], color);
    loadDebianLogo(&logos[5], color);
    loadManjaroLogo(&logos[6], color);
    loadVoidLogo(&logos[7], color);

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

#endif

