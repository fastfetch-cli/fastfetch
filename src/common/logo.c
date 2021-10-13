#include "fastfetch.h"

#include <string.h>
#include <unistd.h>

static void initLogoUnknown(FFinstance* instance)
{
    instance->config.logo.lines =
        "       ________        \n"
        "   _jgN########Ngg_    \n"
        " _N##N@@\"\"  \"\"9NN##Np_ \n"
        "d###P            N####p\n"
        "\"^^\"              T####\n"
        "                  d###P\n"
        "               _g###@F \n"
        "            _gN##@P    \n"
        "          gN###F\"      \n"
        "         d###F         \n"
        "        0###F          \n"
        "        0###F          \n"
        "        0###F          \n"
        "        \"NN@'          \n"
        "                       \n"
        "         ___           \n"
        "        q###r          \n"
        "         \"\"            ";
}

static void initLogoArch(FFinstance* instance)
{
    instance->config.logo.lines =
        "$1                  -`                 \n"
        "$1                 .o+`                \n"
        "$1                `ooo/                \n"
        "$1               `+oooo:               \n"
        "$1              `+oooooo:              \n"
        "$1              -+oooooo+:             \n"
        "$1            `/:-:++oooo+:            \n"
        "$1           `/++++/+++++++:           \n"
        "$1          `/++++++++++++++:          \n"
        "$1         `/+++ooooooooooooo/`        \n"
        "$1        ./ooosssso++osssssso+`       \n"
        "$1       .oossssso-````/ossssss+`      \n"
        "$1      -osssssso.      :ssssssso.     \n"
        "$1     :osssssss/        osssso+++.    \n"
        "$1    /ossssssss/        +ssssooo/-    \n"
        "$1  `/ossssso+/:-        -:/+osssso+-  \n"
        "$1 `+sso+:-`                 `.-/+oso: \n"
        "$1`++:.                           `-/+/\n"
        "$1.`                                 `/";
    instance->config.logo.colors[0] = "\033[36m"; //cyan
}

static void initLogoArtix(FFinstance* instance)
{
    instance->config.logo.lines =
        "$1                   '                  \n"
        "$1                  'o'                 \n"
        "$1                 'ooo'                \n"
        "$1                'ooxoo'               \n"
        "$1               'ooxxxoo'              \n"
        "$1              'oookkxxoo'             \n"
        "$1             'oiioxkkxxoo'            \n"
        "$1            ':;:iiiioxxxoo'           \n"
        "$1              `'.;::ioxxoo''          \n"
        "$1          '-.      `':;jiooo'         \n"
        "$1        'oooio-..     `'i:io'         \n"
        "$1       'ooooxxxxoio:,.   `'-;'        \n"
        "$1      'ooooxxxxxkkxoooIi:-.  `'       \n"
        "$1     'ooooxxxxxkkkkxoiiiiiji'         \n"
        "$1    'ooooxxxxxkxxoiiii:'`     .i'     \n"
        "$1   'ooooxxxxxoi:::'`       .;ioxo'    \n"
        "$1  'ooooxooi::'`         .:iiixkxxo'   \n"
        "$1 'ooooi:'`                `'';ioxxo'  \n"
        "$1'i:'`                          '':io' \n"
        "$1'`                                  `'";
    instance->config.logo.colors[0] = "\033[36m"; //cyan
}

static void initLogoCelOS(FFinstance* instance)
{
    instance->config.logo.lines =
        "$1             `-:/++++/:-`            \n"
        "$1          -/syyyyyyyyyyyyy+-         \n"
        "$1        :ssssyyyyyyyyyyyyyyyy/       \n"
        "$1      .osy$2mmmmmmmmmmmmmmmNNNNNmmhy+  \n"
        "$1     .sssshhhhhhhddddddddddddddds-   \n"
        "$1    `osssssssyyyyyyyyyyyyyyyyyyhy`   \n"
        "$1    :ssssssyyyyyyyyyyyyyyyyyyyyhh/   \n"
        "$2sMMMMMMMMMMMMMMMMMMMMMMMh$1yyyyyyhho   \n"
        "$1    :sssssssyyyyyyyyyyyyyyyyyyyhh/   \n"
        "$1    `ssssssssyyyyyyyyyyyyyyyyyyhy.   \n"
        "$1     -sssssyddddddddddddddddddddy    \n"
        "$1      -ssss$2hmmmmmmmmmmmmmmmmmmmyssss-\n"
        "$1       `/ssssyyyyyyyyyyyyyyyy+`      \n"
        "$1         `:osyyyyyyyyyyyyys/`        \n"
        "$1            `.:/+ooooo+:-`           ";
    instance->config.logo.colors[0] = "\033[35m"; //magenta
    instance->config.logo.colors[1] = "\033[30m"; //black
}

static void initLogoDebian(FFinstance* instance)
{
    instance->config.logo.lines =
        "$2       _,met$$$$$$$$$$gg.       \n"
        "$2    ,g$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$P.    \n"
        "$2  ,g$$$$P\"         \"\"\"Y$$$$.\". \n"
        "$2 ,$$$$P'               `$$$$$$. \n"
        "$2',$$$$P       ,ggs.     `$$$$b:\n"
        "$2`d$$$$'     ,$P\"'   $1.$2    $$$$$$ \n"
        "$2 $$$$P      d$'     $1,$2    $$$$$$P\n"
        "$2 $$$$:      $$.   $1-$2    ,d$$$$'  \n"
        "$2 $$$$;      Y$b._   _,d$P'   \n"
        "$2 Y$$$$.    $1`.$2`\"Y$$$$$$$$P\"'      \n "
        "$2 `$$$$b      $1\"-.__          \n"
        "$2  `Y$$$$                     \n"
        "$2   `Y$$$$.                   \n"
        "$2     `$$$$b.                 \n"
        "$2       `Y$$$$b.              \n"
        "$2          `\"Y$$b._          \n"
        "$2             `\"\"\"          ";
    instance->config.logo.colors[0] = "\033[31m"; //red
    instance->config.logo.colors[1] = "\033[37m"; //white
}

static void initLogoFedora(FFinstance* instance)
{
    instance->config.logo.lines =
        "$1             .',;::::;,'.                     \n"
        "$1         .';:cccccccccccc:;,.                 \n"
        "$1      .;cccccccccccccccccccccc;.              \n"
        "$1    .:cccccccccccccccccccccccccc:.            \n"
        "$1  .;ccccccccccccc;$2        $1;ccccccc;.      \n"
        "$1 .:ccccccccccccc;$2          $1;ccccccc:.     \n"
        "$1.:ccccccccccccc;$2    $1;cc;$2    $1;ccccccc:.\n"
        "$1,cccccccccccccc;$2    $1;cc;$2    $1;cccccccc,\n"
        "$1:cccccccccccccc;$2    $1;cccccccccccccccc:    \n"
        "$1:ccccccc;$2      $1;$2        $1;cccccccccccc:\n"
        "$1cccccc;$2        $1;$2        $1;cccccccccccc;\n"
        "$1ccccc;$2    $1;cccc;$2    $1;cccccccccccccccc'\n"
        "$1ccccc;$2   $1;ccccc;$2    $1;ccccccccccccccc; \n"
        "$1ccccc;$2     $1ccc$2     $1;ccccccccccccccc;  \n"
        "$1cccccc;$2           $1;cccccccccccccc:,       \n"
        "$1cccccccc;$2       $1;cccccccccccccc:,.        \n"
        "$1ccccccccccccccccccccccccccccc:'.              \n"
        "$1:ccccccccccccccccccccccc:;,..                 \n"
        "$1 ':cccccccccccccccc::;,.                      ";
    instance->config.logo.colors[0] = "\033[34m"; //blue
    instance->config.logo.colors[1] = "\033[37m"; //white
}

static void initLogoGaruda(FFinstance* instance)
{
    instance->config.logo.lines =
        "$1                   .%;888:8898898:            \n"
        "$1                 x;XxXB%89b8:b8%b88:          \n"
        "$1              .8Xxd                8X:.       \n"
        "$1            .8Xx;                    8x:.     \n"
        "$1          .tt8x          .d            x88;   \n"
        "$1       .@8x8;          .db:              xx@; \n"
        "$1     ,tSXX°          .bbbbbbbbbbbbbbbbbbbB8x@;\n"
        "$1   .SXxx            bBBBBBBBBBBBBBBBBBBBbSBX8;\n"
        "$1 ,888S                                     pd!\n"
        "$18X88/                                       q \n"
        "$18X88/                                         \n"
        "$1GBB.                                          \n"
        "$1 x%88        d888@8@X@X@X88X@@XX@@X@8@X.      \n"
        "$1   dxXd    dB8b8b8B8B08bB88b998888b88x.       \n"
        "$1    dxx8o                      .@@;.          \n"
        "$1      dx88                   .t@x.            \n"
        "$1        d:SS@8ba89aa67a853Sxxad.              \n"
        "$1          .d988999889889899dd.                ";
    instance->config.logo.colors[0] = "\033[31m"; //red
}

static void initLogoGentoo(FFinstance* instance)
{
    instance->config.logo.lines =
        "$1         -/oyddmdhs+:.             \n"
        "$1     -o$2dNMMMMMMMMNNmhy+$1-`          \n"
        "$1   -y$2NMMMMMMMMMMMNNNmmdhy$1+-        \n"
        "$1 `o$2mMMMMMMMMMMMMNmdmmmmddhhy$1/`     \n"
        "$1 om$2MMMMMMMMMMMN$1hhyyyo$2hmdddhhhd$1o`   \n"
        "$1.y$2dMMMMMMMMMMd$1hs++so/s$2mdddhhhhdm$1+` \n"
        "$1 oy$2hdmNMMMMMMMN$1dyooy$2dmddddhhhhyhN$1d.\n"
        "$1  :o$2yhhdNNMMMMMMMNNNmmdddhhhhhyym$1Mh\n"
        "$1    .:$2+sydNMMMMMNNNmmmdddhhhhhhmM$1my\n"
        "$1       /m$2MMMMMMNNNmmmdddhhhhhmMNh$1s:\n"
        "$1    `o$2NMMMMMMMNNNmmmddddhhdmMNhs$1+` \n"
        "$1  `s$2NMMMMMMMMNNNmmmdddddmNMmhs$1/.   \n"
        "$1 /N$2MMMMMMMMNNNNmmmdddmNMNdso$1:`     \n"
        "$1+M$2MMMMMMNNNNNmmmmdmNMNdso$1/-        \n"
        "$1yM$2MNNNNNNNmmmmmNNMmhs+/$1-`          \n"
        "$1/h$2MMNNNNNNNNMNdhs++/$1-`             \n"
        "$1`/$2ohdmmddhys+++/:$1.`                \n"
        "$1  `-//////:--.                     ";
    instance->config.logo.colors[0] = "\033[35m"; //magenta
    instance->config.logo.colors[1] = "\033[37m"; //white
}

static void initLogoManjaro(FFinstance* instance)
{
    instance->config.logo.lines =
        "$1██████████████████  ████████\n"
        "$1██████████████████  ████████\n"
        "$1██████████████████  ████████\n"
        "$1██████████████████  ████████\n"
        "$1████████            ████████\n"
        "$1████████  ████████  ████████\n"
        "$1████████  ████████  ████████\n"
        "$1████████  ████████  ████████\n"
        "$1████████  ████████  ████████\n"
        "$1████████  ████████  ████████\n"
        "$1████████  ████████  ████████\n"
        "$1████████  ████████  ████████\n"
        "$1████████  ████████  ████████\n"
        "$1████████  ████████  ████████";
    instance->config.logo.colors[0] = "\033[32m"; //green
}

static void initLogoMint(FFinstance* instance)
{
    instance->config.logo.lines =
        "$2             ...-:::::-...              \n"
        "$2          .-MMMMMMMMMMMMMMM-.           \n"
        "$2      .-MMMM$1`..-:::::::-..`$2MMMM-.       \n"
        "$2    .:MMMM$1.:MMMMMMMMMMMMMMM:.$2MMMM:.     \n"
        "$2   -MMM$1-M---MMMMMMMMMMMMMMMMMMM.$2MMM-    \n"
        "$2 `:MMM$1:MM`  :MMMM:....::-...-MMMM:$2MMM:` \n"
        "$2 :MMM$1:MMM`  :MM:`  ``    ``  `:MMM:$2MMM: \n"
        "$2.MMM$1.MMMM`  :MM.  -MM.  .MM-  `MMMM.$2MMM.\n"
        "$2:MMM$1:MMMM`  :MM.  -MM-  .MM:  `MMMM-$2MMM:\n"
        "$2:MMM$1:MMMM`  :MM.  -MM-  .MM:  `MMMM:$2MMM:\n"
        "$2:MMM$1:MMMM`  :MM.  -MM-  .MM:  `MMMM-$2MMM:\n"
        "$2.MMM$1.MMMM`  :MM:--:MM:--:MM:  `MMMM.$2MMM.\n"
        "$2 :MMM$1:MMM-  `-MMMMMMMMMMMM-`  -MMM-$2MMM: \n"
        "$2  :MMM$1:MMM:`                `:MMM:$2MMM:  \n"
        "$2   .MMM$1.MMMM:--------------:MMMM.$2MMM.   \n"
        "$2     '-MMMM$1.-MMMMMMMMMMMMMMM-.$2MMMM-'    \n"
        "$2       '.-MMMM$1``--:::::--``$2MMMM-.'      \n"
        "$2            '-MMMMMMMMMMMMM-'           \n"
        "$2               ``-:::::-``              ";
    instance->config.logo.colors[0] = "\033[32m"; //green
    instance->config.logo.colors[1] = "\033[37m"; //white
}

static void initLogoPop(FFinstance* instance)
{
    instance->config.logo.lines =
        "$1             /////////////             \n"
        "$1         /////////////////////         \n"
        "$1      ///////$2*767$1////////////////      \n"
        "$1    //////$27676767676*$1//////////////    \n"
        "$1   /////$276767$1//$27676767$1//////////////   \n"
        "$1  /////$2767676$1///$2*76767$1///////////////  \n"
        "$1 ///////$2767676$1///$276767$1.///$27676*$1/////// \n"
        "$1/////////$2767676$1//$276767$1///$2767676$1////////\n"
        "$1//////////$276767676767$1////$276767$1/////////\n"
        "$1///////////$276767676$1//////$27676$1//////////\n"
        "$1////////////,$27676$1,///////$2767$1///////////\n"
        "$1/////////////*$27676$1///////$276$1////////////\n"
        "$1///////////////$27676$1////////////////////\n"
        "$1 ///////////////$27676$1///$2767$1//////////// \n"
        "$1  //////////////////////$2'$1////////////  \n"
        "$1   //////$2.7676767676767676767,$1//////   \n"
        "$1    /////$2767676767676767676767$1/////    \n"
        "$1      ///////////////////////////      \n"
        "$1         /////////////////////         \n"
        "$1             /////////////             ";
    instance->config.logo.colors[0] = "\033[36m"; //cyan
    instance->config.logo.colors[1] = "\033[37m"; //white
}

static void initLogoUbuntu(FFinstance* instance)
{
    instance->config.logo.lines =
        "$1             .-/+oossssoo+/-.           \n"
        "$1         `:+ssssssssssssssssss+:`       \n"
        "$1       -+ssssssssssssssssssyyssss+-     \n"
        "$1     .ossssssssssssssssssd$2MMMNy$1sssso.   \n"
        "$1   /sssssssssss$2hdmmNNmmyNMMMMh$1ssssss/   \n"
        "$1  +sssssssss$2hmydMMMMMMMNddddy$1ssssssss+  \n"
        "$1 /ssssssss$2hNMMMyhhyyyyhmNMMMNh$1ssssssss/ \n"
        "$1.ssssssss$2dMMMNh$1ssssssssss$2hNMMMd$1ssssssss.\n"
        "$1+ssss$2hhhyNMMNy$1ssssssssssss$2yNMMMy$1sssssss+\n"
        "$1oss$2yNMMMNyMMh$1ssssssssssssss$2hmmmh$1ssssssso\n"
        "$1oss$2yNMMMNyMMh$1ssssssssssssss$2hmmmh$1ssssssso\n"
        "$1+ssss$2hhhyNMMNy$1ssssssssssss$2yNMMMy$1sssssss+\n"
        "$1.ssssssss$2dMMMNh$1ssssssssss$2hNMMMd$1ssssssss.\n"
        "$1 /ssssssss$2hNMMMyhhyyyyhdNMMMNh$1ssssssss/ \n"
        "$1  +sssssssss$2dmydMMMMMMMMddddy$1ssssssss+  \n"
        "$1   /sssssssssss$2hdmNNNNmyNMMMMh$1ssssss/   \n"
        "$1    .ossssssssssssssssss$2dMMMNy$1sssso.    \n"
        "$1     -+sssssssssssssssss$2yyy$1ssss+-       \n"
        "$1       `:+ssssssssssssssssss+:`         \n"
        "$1           .-/+oossssoo+/-.             ";
    instance->config.logo.colors[0] = "\033[31m"; //red
    instance->config.logo.colors[1] = "\033[37m"; //white
}

static void initLogoVoid(FFinstance* instance)
{
    instance->config.logo.lines =
        "$1                __.;=====;.__                \n"
        "$1            _.=+==++=++=+=+===;.             \n"
        "$1             -=+++=+===+=+=+++++=_           \n"
        "$1        .     -=:``     `--==+=++==.         \n"
        "$1       _vi,    `            --+=++++:        \n"
        "$1      .uvnvi.       _._       -==+==+.       \n"
        "$1     .vvnvnI`    .;==|==;.     :|=||=|.      \n"
        "$2+QmQQm$1pvvnv;$2 _yYsyQQWUUQQQm #QmQ#$1:$2QQQWUV$QQm.\n"
        "$2 -QQWQW$1pvvo$2wZ?.wQQQE$1==<$2QWWQ/QWQW.QQWW$1(:$2 jQWQE\n"
        "$2  -$QQQQmmU'  jQQQ$1@+=<$2QWQQ)mQQQ.mQQQC$1+;$2jWQQ@'\n"
        "$2   -$WQ8Y$1nI:$2   QWQQwgQQWV$1`$2mWQQ.jQWQQgyyWW@!  \n"
        "$1     -1vvnvv.     `~+++`        ++|+++       \n"
        "$1      +vnvnnv,                 `-|===        \n"
        "$1       +vnvnvns.           .      :=-        \n"
        "$1        -Invnvvnsi..___..=sv=.     `         \n"
        "$1          +Invnvnvnnnnnnnnvvnn;.             \n"
        "$1            ~|Invnvnvvnvvvnnv}+`             \n"
        "$1               -~|{*l}*|~                    ";
    instance->config.logo.colors[0] = "\033[32m"; //green
    instance->config.logo.colors[1] = "\033[30m"; //black
}

static bool loadLogoSet(FFinstance* instance, const char* logo)
{
    if(instance->config.logo.freeable)
        free(instance->config.logo.lines);

    instance->config.logo.freeable = false;
    instance->config.logo.allLinesSameLength = true;

    if(logo == NULL || *logo == '\0')
        return false;
    else if(strcasecmp(logo, "none") == 0)
    {
        instance->config.logo.lines = "";
        instance->config.logoKeySpacing = 0; //This is wanted in most cases, so just set it. None logo is set in src/common/init.c
    }
    else if(strcasecmp(logo, "unknown") == 0)
        initLogoUnknown(instance);
    else if(strcasecmp(logo, "arch") == 0)
        initLogoArch(instance);
    else if(strcasecmp(logo, "artix") == 0)
        initLogoArtix(instance);
    else if(strcasecmp(logo, "celos") == 0)
        initLogoCelOS(instance);
    else if(strcasecmp(logo, "debian") == 0)
        initLogoDebian(instance);
    else if(strcasecmp(logo, "fedora") == 0)
        initLogoFedora(instance);
    else if(strcasecmp(logo, "garuda") == 0)
        initLogoGaruda(instance);
    else if(strcasecmp(logo, "gentoo") == 0)
        initLogoGentoo(instance);
    else if(strcasecmp(logo, "manjaro") == 0)
        initLogoManjaro(instance);
    else if(strcasecmp(logo, "mint") == 0)
        initLogoMint(instance);
    else if(strcasecmp(logo, "pop") == 0 || strcasecmp(logo, "pop_os") == 0)
        initLogoPop(instance);
    else if(strcasecmp(logo, "ubuntu") == 0)
        initLogoUbuntu(instance);
    else if(strcasecmp(logo, "void") == 0)
        initLogoVoid(instance);
    else
        return false;

    return true;
}

void ffLoadLogoSet(FFinstance* instance, const char* logo)
{
    if(loadLogoSet(instance, logo))
        return;

    FFstrbuf logoChars;
    ffStrbufInitA(&logoChars, 1024);

    if(!ffGetFileContent(logo, &logoChars))
    {
        ffStrbufDestroy(&logoChars);
        if(instance->config.showErrors)
            printf(FASTFETCH_TEXT_MODIFIER_ERROR"Error: unknown logo / logo file not found: %s"FASTFETCH_TEXT_MODIFIER_RESET"\n", logo);
        initLogoUnknown(instance);
        return;
    }

    instance->config.logo.allLinesSameLength = false;
    instance->config.logo.freeable = true;
    instance->config.logo.lines = logoChars.chars;
}

void ffLoadLogo(FFinstance* instance)
{
    const FFOSResult* result = ffDetectOS(instance);

    if(
        !loadLogoSet(instance, result->name.chars) &&
        !loadLogoSet(instance, result->id.chars) &&
        !loadLogoSet(instance, result->systemName.chars) &&
        !loadLogoSet(instance, result->idLike.chars)
    ) initLogoUnknown(instance);
}

static uint32_t strLengthUTF8(const char* str, uint32_t bytesLength)
{
    uint32_t length = 0;

    for (uint32_t i = 0; i < bytesLength; ++i)
    {
        int c = (unsigned char) str[i];

        if((c>=0 && c<=127))        i += 0;
        else if((c & 0xE0) == 0xC0) i += 1;
        else if((c & 0xF0) == 0xE0) i += 2;
        else if((c & 0xF8) == 0xF0) i += 3;
        else return 0; //invalid utf8

        ++length;
    }

    return length;
}

void ffPrintLogoLine(FFinstance* instance)
{
    for(int16_t i = 0; i < instance->config.offsetx; i++)
        putchar(' ');

    if(*instance->config.logo.lines == '\0')
    {
        for(uint32_t i = 0; i < instance->state.logoWidth; ++i)
            putchar(' ');
        return;
    }

    if(*instance->config.logo.lines == '\n')
    {
        ++instance->config.logo.lines;
        return;
    }

    const char* start = instance->config.logo.lines;
    uint32_t colorPlaceholdersLength = 0;

    uint32_t cutValue = instance->config.offsetx < 0 ? (uint32_t) (instance->config.offsetx * -1) : 0;
    uint32_t cut = cutValue;

    fputs(FASTFETCH_TEXT_MODIFIER_BOLT, stdout);

    while(*instance->config.logo.lines != '\n' && *instance->config.logo.lines != '\0')
    {
        char current = *instance->config.logo.lines;
        ++instance->config.logo.lines;

        if(current != '$')
        {
            if(cut > 0) --cut;
            else putchar(current);
            continue;
        }

        current = *instance->config.logo.lines;
        ++instance->config.logo.lines;

        if(current == '\n' || current == '\0')
        {
            if(cut == 0)
                putchar('$');
            break;
        }

        if(current == '$')
        {
            ++colorPlaceholdersLength;
            if(cut > 0) --cut;
            else putchar('$');
            continue;
        }

        int index = ((int) current) - 49;

        if(index < 0 || index > 9)
        {
            if(cut > 0) --cut;
            else putchar('$');

            if(cut > 0) --cut;
            else putchar(current);

            continue;
        }

        colorPlaceholdersLength += 2;

        if(!instance->config.colorLogo)
            continue;

        fputs(instance->config.logo.colors[index], stdout);
    }

    fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);

    const uint32_t logoKeySpacing = cut > instance->config.logoKeySpacing ? 0 : instance->config.logoKeySpacing - cut;
    for(uint32_t i = 0; i < logoKeySpacing; ++i)
        putchar(' ');

    if(instance->state.logoWidth == 0 || !instance->config.logo.allLinesSameLength)
    {
        uint32_t lineLength = strLengthUTF8(start, (uint32_t) (instance->config.logo.lines - start)) - colorPlaceholdersLength + instance->config.logoKeySpacing;

        if(cutValue > lineLength)
            lineLength = 0;
        else
            lineLength -= cutValue;

        if(instance->state.logoWidth < lineLength)
            instance->state.logoWidth = lineLength;
    }

    if(*instance->config.logo.lines == '\n')
        ++instance->config.logo.lines;
}

void ffPrintRemainingLogo(FFinstance* instance)
{
    while(*instance->config.logo.lines != '\0')
    {
        ffPrintLogoLine(instance);
        putchar('\n');
    }
}

#ifndef FASTFETCH_BUILD_FLASHFETCH

void ffListLogos()
{
    puts(
        "none\n"
        "unknown\n"
        "arch\n"
        "artix\n"
        "celos\n"
        "debian\n"
        "fedora\n"
        "garuda\n"
        "gentoo\n"
        "manjaro\n"
        "mint\n"
        "pop\n"
        "ubuntu\n"
        "void"
    );
}

void ffPrintLogos(FFinstance* instance)
{
    #define FF_LOGO_PRINT(name, loadFunction) \
        loadFunction(instance); \
        printf(FASTFETCH_TEXT_MODIFIER_BOLT"%s" #name FASTFETCH_TEXT_MODIFIER_RESET":\n", instance->config.colorLogo ? instance->config.logo.colors[0] : ""); \
        ffPrintRemainingLogo(instance); \
        putchar('\n');

    FF_LOGO_PRINT(unknown, initLogoUnknown)
    FF_LOGO_PRINT(arch, initLogoArch)
    FF_LOGO_PRINT(artix, initLogoArtix)
    FF_LOGO_PRINT(celos, initLogoCelOS)
    FF_LOGO_PRINT(debian, initLogoDebian)
    FF_LOGO_PRINT(fedora, initLogoFedora)
    FF_LOGO_PRINT(garuda, initLogoGaruda)
    FF_LOGO_PRINT(gentoo, initLogoGentoo)
    FF_LOGO_PRINT(manjaro, initLogoManjaro)
    FF_LOGO_PRINT(mint, initLogoMint)
    FF_LOGO_PRINT(pop, initLogoPop)
    FF_LOGO_PRINT(ubuntu, initLogoUbuntu)
    FF_LOGO_PRINT(void, initLogoVoid)

    #undef FF_LOGO_PRINT
}

#endif

