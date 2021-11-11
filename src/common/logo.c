#include "fastfetch.h"

#include <limits.h>
#include <string.h>
#include <unistd.h>

#define FF_LOGO_INIT static FFlogo logo; static bool init = false; if(init) return &logo; init = true; logo.isFromUser = false;
#define FF_LOGO_NAMES(...) static const char* names[] = (const char*[]) { __VA_ARGS__, NULL }; logo.names = names;
#define FF_LOGO_LINES(x) logo.lines = x;
#define FF_LOGO_COLORS(...) static const char* colors[] = (const char*[]) { __VA_ARGS__, NULL }; logo.colors = colors;
#define FF_LOGO_RETURN return &logo;

static const FFlogo* getLogoUnknown()
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("unkown", "question mark")
    FF_LOGO_LINES(
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
        "         \"\"            "
    )
    FF_LOGO_COLORS("")
    FF_LOGO_RETURN
}

static const FFlogo* getLogoNone()
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("none", "empty", "")
    FF_LOGO_LINES("")
    FF_LOGO_COLORS("")
    FF_LOGO_RETURN
}

static const FFlogo* getLogoArch()
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("arch", "archlinux", "arch-linux")
    FF_LOGO_LINES(
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
    )
    FF_LOGO_COLORS(
        "\033[36m" //cyan
    )
    FF_LOGO_RETURN
}

static const FFlogo* getLogoArtix()
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("artix", "artixlinux", "artix-linux")
    FF_LOGO_LINES(
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
    )
    FF_LOGO_COLORS(
        "\033[36m" //cyan
    )
    FF_LOGO_RETURN
}

static const FFlogo* getLogoCelOS()
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("celos", "celos-linux")
    FF_LOGO_LINES(
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
        "$1            `.:/+ooooo+:-`           "
    )
    FF_LOGO_COLORS(
        "\033[35m", //magenta
        "\033[30m" //black
    )
    FF_LOGO_RETURN
}

static const FFlogo* getLogoCentOS()
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("centos", "centos-linux")
    FF_LOGO_LINES(
        "$1                 ..                 \n"
        "$1               .PLTJ.               \n"
        "$1              <><><><>              \n"
        "$2     KKSSV' 4KKK $1LJ$4 KKKL.'VSSKK     \n"
        "$2     KKV' 4KKKKK $1LJ$4 KKKKAL 'VKK     \n"
        "$2     V' ' 'VKKKK $1LJ$4 KKKKV' ' 'V     \n"
        "$2     .4MA.' 'VKK $1LJ$4 KKV' '.4Mb.     \n"
        "$4   . $2KKKKKA.' 'V $1LJ$4 V' '.4KKKKK $3.   \n"
        "$4 .4D $2KKKKKKKA.'' $1LJ$4 ''.4KKKKKKK $3FA. \n"
        "$4<QDD ++++++++++++  $3++++++++++++ GFD>\n"
        "$4 'VD $3KKKKKKKK'.. $2LJ $1..'KKKKKKKK $3FV  \n"
        "$4   ' $3VKKKKK'. .4 $2LJ $1K. .'KKKKKV $3'   \n"
        "$3      'VK'. .4KK $2LJ $1KKA. .'KV'      \n"
        "$3     A. . .4KKKK $2LJ $1KKKKA. . .4     \n"
        "$3     KKA. 'KKKKK $2LJ $1KKKKK' .4KK     \n"
        "$3     KKSSA. VKKK $2LJ $1KKKV .4SSKK     \n"
        "$2              <><><><>              \n"
        "$1               'MKKM'               \n"
        "$1                 ''                 "
    )
    FF_LOGO_COLORS(
        "\033[33m", //yellow
        "\033[32m", //green
        "\033[34m", //blue
        "\033[35m", //magenta
        "\033[37m" //white
    )
    FF_LOGO_RETURN
}

static const FFlogo* getLogoDebian()
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("debian", "debian-linux")
    FF_LOGO_LINES(
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
        "$2             `\"\"\"          "
    )
    FF_LOGO_COLORS(
        "\033[31m", //red
        "\033[37m" //white
    )
    FF_LOGO_RETURN
}

static const FFlogo* getLogoFedora()
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("fedora", "fedora-linux")
    FF_LOGO_LINES(
        "$1             .',;::::;,'.             \n"
        "$1         .';:cccccccccccc:;,.         \n"
        "$1      .;cccccccccccccccccccccc;.      \n"
        "$1    .:cccccccccccccccccccccccccc:.    \n"
        "$1  .;ccccccccccccc;$2.:dddl:.$1;ccccccc;.  \n"
        "$1 .:ccccccccccccc;$2OWMKOOXMWd$1;ccccccc:. \n"
        "$1.:ccccccccccccc;$2KMMc$1;cc;$2xMMc$1;ccccccc:.\n"
        "$1,cccccccccccccc;$2MMM.$1;cc;$2;WW:$1;cccccccc,\n"
        "$1:cccccccccccccc;$2MMM.$1;cccccccccccccccc:\n"
        "$1:ccccccc;$2oxOOOo$1;$2MMM000k.$1;cccccccccccc:\n"
        "$1cccccc;$20MMKxdd:$1;$2MMMkddc.$1;cccccccccccc;\n"
        "$1ccccc;$2XMO'$1;cccc;$2MMM.$1;cccccccccccccccc'\n"
        "$1ccccc;$2MMo$1;ccccc;$2MMW.$1;ccccccccccccccc; \n"
        "$1ccccc;$20MNc.$1ccc$2.xMMd$1;ccccccccccccccc;  \n"
        "$1cccccc;$2dNMWXXXWM0:$1;cccccccccccccc:,   \n"
        "$1cccccccc;$2.:odl:.$1;cccccccccccccc:,.    \n"
        "$1ccccccccccccccccccccccccccccc:'.      \n"
        "$1:ccccccccccccccccccccccc:;,..         \n"
        "$1 ':cccccccccccccccc::;,.              "
    )
    FF_LOGO_COLORS(
        "\033[34m", //blue
        "\033[37m" //white
    )
    FF_LOGO_RETURN
}

static const FFlogo* getLogoFedoraOld()
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("fedora_old", "fedora-old", "fedora-linux-old", "fedora-linux_old")
    FF_LOGO_LINES(
        "$1          /:-------------:\\       \n"
        "$1       :-------------------::     \n"
        "$1     :-----------$2/shhOHbmp$1---:\\   \n"
        "$1   /-----------$2omMMMNNNMMD$1  ---:  \n"
        "$1  :-----------$2sMMMMNMNMP$1.    ---: \n"
        "$1 :-----------$2:MMMdP$1-------    ---\\\n"
        "$1,------------$2:MMMd$1--------    ---:\n"
        "$1:------------$2:MMMd$1-------    .---:\n"
        "$1:----    $2oNMMMMMMMMMNho$1     .----:\n"
        "$1:--     .$2+shhhMMMmhhy++$1   .------/\n"
        "$1:-    -------$2:MMMd$1--------------: \n"
        "$1:-   --------$2/MMMd$1-------------;  \n"
        "$1:-    ------$2/hMMMy$1------------:   \n"
        "$1:--$2 :dMNdhhdNMMNo$1------------;    \n"
        "$1:---$2:sdNMMMMNds:$1------------:     \n"
        "$1:------$2:://:$1-------------::       \n"
        "$1:---------------------://         "
    )
    FF_LOGO_COLORS(
        "\033[34m", //blue
        "\033[37m" //white
    )
    FF_LOGO_RETURN
}

static const FFlogo* getLogoGaruda()
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("garuda", "garuda-linux")
    FF_LOGO_LINES(
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
        "$1          .d988999889889899dd.                "
    )
    FF_LOGO_COLORS(
        "\033[31m" //red
    )
    FF_LOGO_RETURN
}

static const FFlogo* getLogoGentoo()
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("gentoo", "gentoo-linux")
    FF_LOGO_LINES(
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
        "$1  `-//////:--.                     "
    )
    FF_LOGO_COLORS(
        "\033[35m", //magenta
        "\033[37m" //white
    )
    FF_LOGO_RETURN
}

static const FFlogo* getLogoManjaro()
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("manjaro", "manjaro-linux")
    FF_LOGO_LINES(
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
        "$1████████  ████████  ████████"
    )
    FF_LOGO_COLORS(
        "\033[32m" //green
    )
    FF_LOGO_RETURN
}

static const FFlogo* getLogoMint()
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("mint", "mint-linux", "linux-mint")
    FF_LOGO_LINES(
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
        "$2               ``-:::::-``              "
    )
    FF_LOGO_COLORS(
        "\033[32m", //green
        "\033[37m" //white
    )
    FF_LOGO_RETURN
}

static const FFlogo* getLogoMintOld()
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("mint_old", "mint-old", "mint-linux_old", "mint-linux-old", "linux-mint_old", "linux-mint-old")
    FF_LOGO_LINES(
        "$1MMMMMMMMMMMMMMMMMMMMMMMMMmds+.     \n"
        "$1MMm----::-://////////////oymNMd+`  \n"
        "$1MMd      $2/++                $1-sNMd: \n"
        "$1MMNso/`  $2dMM    `.::-. .-::.` $1.hMN:\n"
        "$1ddddMMh  $2dMM   :hNMNMNhNMNMNh: $1`NMm\n"
        "$1    NMm  $2dMM  .NMN/-+MMM+-/NMN` $1dMM\n"
        "$1    NMm  $2dMM  -MMm  `MMM   dMM. $1dMM\n"
        "$1    NMm  $2dMM  -MMm  `MMM   dMM. $1dMM\n"
        "$1    NMm  $2dMM  .mmd  `mmm   yMM. $1dMM\n"
        "$1    NMm  $2dMM`  ..`   ...   ydm. $1dMM\n"
        "$1    hMM- $2+MMd/-------...-:sdds  $1dMM\n"
        "$1    -NMm- $2:hNMNNNmdddddddddy/`  $1dMM\n"
        "$1    -dMNs-$2``-::::-------.``    $1dMM \n"
        "$1    `/dMNmy+/:-------------:/yMMM  \n"
        "$1      ./ydNMMMMMMMMMMMMMMMMMMMMM   \n"
        "$1          .MMMMMMMMMMMMMMMMMMM     "
    )
    FF_LOGO_COLORS(
        "\033[32m", //green
        "\033[37m" //white
    )
    FF_LOGO_RETURN
}

static const FFlogo* getLogoPop()
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("pop", "pop-linux")
    FF_LOGO_LINES(
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
        "$1             /////////////             "
    )
    FF_LOGO_COLORS(
        "\033[36m", //cyan
        "\033[37m" //white
    )
    FF_LOGO_RETURN
}

static const FFlogo* getLogoUbuntu()
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("ubuntu", "ubuntu-linux")
    FF_LOGO_LINES(
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
        "$1           .-/+oossssoo+/-.             "
    )
    FF_LOGO_COLORS(
        "\033[31m", //red
        "\033[37m" //white
    )
    FF_LOGO_RETURN
}

static const FFlogo* getLogoVoid()
{
    FF_LOGO_INIT
    FF_LOGO_NAMES("void", "void-linux")
    FF_LOGO_LINES(
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
        "$1               -~|{*l}*|~                    "
    )
    FF_LOGO_COLORS(
        "\033[32m", //green
        "\033[30m" //black
    )
    FF_LOGO_RETURN
}

typedef const FFlogo*(*GetLogoMethod)();

static GetLogoMethod* getLogoMethods()
{
    static GetLogoMethod logoMethods[] = {
        getLogoNone,
        getLogoUnknown,
        getLogoArch,
        getLogoArtix,
        getLogoCelOS,
        getLogoCentOS,
        getLogoDebian,
        getLogoFedora,
        getLogoFedoraOld,
        getLogoGaruda,
        getLogoGentoo,
        getLogoManjaro,
        getLogoMint,
        getLogoMintOld,
        getLogoPop,
        getLogoUbuntu,
        getLogoVoid,
        NULL
    };

    return logoMethods;
}

static bool logoHasName(const FFlogo* logo, const char* name)
{
    const char** logoName = logo->names;

    while(*logoName != NULL)
    {
        if(strcasecmp(*logoName, name) == 0)
            return true;

        ++logoName;
    }

    return false;
}

static inline void setLogo(FFinstance* instance, const FFlogo* logo)
{
    instance->config.logo = logo;
    instance->state.logoWidth = 0;
    instance->state.logoLinesIndex = logo->lines;
}

static bool loadLogoSet(FFinstance* instance, const char* name)
{
    GetLogoMethod* logoMethod = getLogoMethods();

    while(*logoMethod != NULL)
    {
        const FFlogo* logo = (*logoMethod)();
        if(logoHasName(logo, name))
        {
            setLogo(instance, logo);
            return true;
        }

        ++logoMethod;
    }

    return false;
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
        instance->config.logo = getLogoUnknown();
        return;
    }

    static FFlogo dummyLogo;
    dummyLogo.isFromUser = true;
    dummyLogo.colors = getLogoNone()->colors;
    dummyLogo.lines = logoChars.chars;
    static const char* NULLSTR = NULL;
    dummyLogo.names = &NULLSTR;

    setLogo(instance, &dummyLogo);
}

static bool loadLogoSetWithVersion(FFinstance* instance, const FFstrbuf* versionID, const FFstrbuf* name)
{
    bool isFedora = logoHasName(getLogoFedora(), name->chars);
    bool isMint   = logoHasName(getLogoMint(),   name->chars);

    if(versionID->length == 0 || (
        !isFedora &&
        !isMint
    )) return loadLogoSet(instance, name->chars);

    long version = strtol(versionID->chars, NULL, 10);

    #define FF_LOAD_LOGO_WITH_VERSION(ver, newLogo, oldLogo) setLogo(instance, (version == 0 || version == LONG_MAX || version == LONG_MIN || version > ver) ? newLogo : oldLogo);

    if(isFedora)
        FF_LOAD_LOGO_WITH_VERSION(34, getLogoFedora(), getLogoFedoraOld())
    else if(isMint)
        FF_LOAD_LOGO_WITH_VERSION(19, getLogoMint(), getLogoMintOld())

    #undef FF_LOAD_LOGO_WITH_VERSION

    return true;
}

void ffLoadLogo(FFinstance* instance)
{
    const FFOSResult* result = ffDetectOS(instance);

    if(
        !loadLogoSetWithVersion(instance, &result->versionID, &result->name) &&
        !loadLogoSetWithVersion(instance, &result->versionID, &result->id) &&
        !loadLogoSetWithVersion(instance, &result->versionID, &result->systemName) &&
        !loadLogoSetWithVersion(instance, &result->versionID, &result->idLike)
    ) setLogo(instance, getLogoUnknown());
}

static uint32_t strLengthUTF8(const char* str, uint32_t bytesLength)
{
    uint32_t length = 0;

    for (uint32_t i = 0; i < bytesLength; ++i)
    {
        int c = (unsigned char) str[i];

        if(c<=127) {}            // i += 0;
        else if((c & 0xE0) == 0xC0) i += 1;
        else if((c & 0xF0) == 0xE0) i += 2;
        else if((c & 0xF8) == 0xF0) i += 3;
        else return 0; //invalid utf8

        ++length;
    }

    return length;
}

#define LOGO_LINE_PRINT_CHAR(c, cut) \
    if(cut == 0) \
        putchar(c); \
    else \
        --cut;

void ffPrintLogoLine(FFinstance* instance)
{
    //If offset x is positive, print it as whitespaces left from the logo
    for(int16_t i = 0; i < instance->config.offsetx; i++)
        putchar(' ');

    //If we have more informations than lines in the logo, print whitespaces.
    //We can return after this, since logoWidth includes logoKeySpacing.
    if(*instance->state.logoLinesIndex == '\0')
    {
        for(uint32_t i = 0; i < instance->state.logoWidth; ++i)
            putchar(' ');
        return;
    }

    //Save the start of the line and the length of color placeholders, so we can calculate the length of the line at the end
    const char* start = instance->state.logoLinesIndex;
    uint32_t colorPlaceholdersLength = 0;


    //If offset x is negative, we will cut the logo. Save the totol amount to cut, and the currently cut amount
    uint32_t cutValue = instance->config.offsetx < 0 ? (uint32_t) (instance->config.offsetx * -1) : 0;
    uint32_t cut = cutValue;

    //Logo is always bold
    fputs(FASTFETCH_TEXT_MODIFIER_BOLT, stdout);

    while(*instance->state.logoLinesIndex != '\n' && *instance->state.logoLinesIndex != '\0')
    {
        //Get the current char
        char current = *instance->state.logoLinesIndex;
        ++instance->state.logoLinesIndex;

        //Not a color placeholder, just print it. User files don't support colors this way.
        if(current != '$' || instance->config.logo->isFromUser)
        {
            LOGO_LINE_PRINT_CHAR(current, cut);
            continue;
        }

        //Skip the dollar sign
        current = *instance->state.logoLinesIndex;
        ++instance->state.logoLinesIndex;

        //We have a dollar sign at the end of the line. Just print it.
        if(current == '\n' || current == '\0')
        {
            if(cut == 0)
                putchar('$');
            break;
        }

        //We have two dollar signs. Print one.
        if(current == '$')
        {
            ++colorPlaceholdersLength;
            LOGO_LINE_PRINT_CHAR('$', cut);
            continue;
        }

        //Map the number to an array index, so that '1' -> 0, '2' -> 1, etc.
        int index = ((int) current) - 49;

        //Invalid index, just print the $ and the following char
        if(index < 0 || index > 9)
        {
            LOGO_LINE_PRINT_CHAR('$', cut);
            LOGO_LINE_PRINT_CHAR(current, cut);
            continue;
        }

        //Index can maximal be 9, so we have exactly two chars for the color
        colorPlaceholdersLength += 2;

        //Print the color, if color support is on.
        if(instance->config.colorLogo)
            fputs(instance->config.logo->colors[index], stdout);
    }

    //Reset out bold logo
    fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);

    //Print the whitespaces between logo and keys. If cut is left, substract it from the spacing. Never go below a spacing of 0.
    const uint32_t logoKeySpacing = cut > instance->config.logoKeySpacing ? 0 : instance->config.logoKeySpacing - cut;
    for(uint32_t i = 0; i < logoKeySpacing; ++i)
        putchar(' ');

    //If we haven't yet calculated the length of the line, do it now
    if(instance->state.logoWidth == 0)
    {
        instance->state.logoWidth = strLengthUTF8(start, (uint32_t) (instance->state.logoLinesIndex - start)) - colorPlaceholdersLength + instance->config.logoKeySpacing;
        instance->state.logoWidth = cutValue > instance->state.logoWidth ? 0 : instance->state.logoWidth - cutValue;
    }

    //If we just finished a line and not the entire logo, skip the newline char
    if(*instance->state.logoLinesIndex == '\n')
        ++instance->state.logoLinesIndex;
}

void ffPrintRemainingLogo(FFinstance* instance)
{
    while(*instance->state.logoLinesIndex != '\0')
    {
        ffPrintLogoLine(instance);
        putchar('\n');
    }
}

#ifndef FASTFETCH_BUILD_FLASHFETCH

void ffPrintLogos(FFinstance* instance)
{
    GetLogoMethod* methods = getLogoMethods();

    while(*methods != NULL)
    {
        instance->config.logo = (*methods)();
        printf(FASTFETCH_TEXT_MODIFIER_BOLT"%s%s"FASTFETCH_TEXT_MODIFIER_RESET":\n", instance->config.colorLogo ? instance->config.logo->colors[0] : "", instance->config.logo->names[0]);
        ffPrintRemainingLogo(instance);
        putchar('\n');
        ++methods;
    }
}

void ffListLogos()
{
    GetLogoMethod* methods = getLogoMethods();

    while(*methods != NULL)
    {
        const FFlogo* logo = (*methods)();

        const char** names = logo->names;

        while(*names != NULL)
        {
            printf("\"%s\" ", *names);
            ++names;
        }

        putchar('\n');

        ++methods;
    }
}

void ffListLogosForAutocompletion()
{
    GetLogoMethod* methods = getLogoMethods();

    while(*methods != NULL)
    {
        printf("%s\n", (*methods)()->names[0]);
        ++methods;
    }
}

#endif
