#include "common/io/io.h"
#include "detection/network/network.h"

#include <net/if.h>
#include <linux/if_arp.h>

const char* ffDetectNetwork(FFinstance* instance, FFlist* result)
{
    FF_STRBUF_AUTO_DESTROY sbType;
    ffStrbufInit(&sbType);

    struct if_nameindex* infs = if_nameindex();
    if(!infs)
        return "if_nameindex() failed";

    FF_STRBUF_AUTO_DESTROY path;
    ffStrbufInit(&path);

    FF_STRBUF_AUTO_DESTROY fileContent;
    ffStrbufInit(&fileContent);

    for(struct if_nameindex* i = infs; !(i->if_index == 0 && i->if_name == NULL); ++i)
    {
        if (strcmp(i->if_name, "lo") == 0)
            continue;

        ffStrbufSetF(&path, "/sys/class/net/%s/operstate", i->if_name);
        if (!ffReadFileBuffer(path.chars, &fileContent))
            continue;

        bool isUp = ffStrbufEqualS(&fileContent, "up");
        if (!isUp && !instance->config.networkAll)
            continue;

        ffStrbufSetF(&path, "/sys/class/net/%s/type", i->if_name);
        if (!ffReadFileBuffer(path.chars, &fileContent))
            continue;

        const char* strType = NULL;
        switch (ffStrbufToUInt16(&fileContent, 0))
        {
            case ARPHRD_ETHER:
            case ARPHRD_EETHER:
                strType = "Ethernet";
                break;
            case ARPHRD_AX25:
                strType = "AX25";
                break;
            case ARPHRD_PRONET:
                strType = "TokenRing";
                break;
            case ARPHRD_CHAOS:
                strType = "Chaosnet";
                break;
            case ARPHRD_IEEE802:
                strType = "IEEE802";
                break;
            case ARPHRD_ARCNET:
                strType = "ARCnet";
                break;
            case ARPHRD_APPLETLK:
                strType = "APPLEtalk";
                break;
            case ARPHRD_DLCI:
                strType = "DLCI";
                break;
            case ARPHRD_ATM:
                strType = "ATM";
                break;
            case ARPHRD_METRICOM:
                strType = "Metricom";
                break;
            case ARPHRD_IEEE1394:
                strType = "IEEE1394";
                break;
            case ARPHRD_EUI64:
                strType = "EUI64";
                break;
            case ARPHRD_INFINIBAND:
                strType = "InfiniBand";
                break;
            default:
                strType = "Other";
                break;
        }

        if (instance->config.networkType.length && !ffStrbufContainIgnCaseS(&instance->config.networkType, strType))
            continue;

        FFNetworkResult* item = (FFNetworkResult*) ffListAdd(result);
        ffStrbufInitS(&item->type, strType);
        ffStrbufInitS(&item->name, i->if_name);
        ffStrbufInit(&item->address);
        item->mtu = 0;
        item->up = isUp;

        ffStrbufSetF(&path, "/sys/class/net/%s/address", i->if_name);
        ffReadFileBuffer(path.chars, &item->address);

        ffStrbufSetF(&path, "/sys/class/net/%s/mtu", i->if_name);
        if (ffReadFileBuffer(path.chars, &fileContent))
            item->mtu = ffStrbufToUInt16(&fileContent, 0);
    }

    if_freenameindex(infs);

    if (result->length == 0)
        return "No network interfaces found";

    return NULL;
}
