#include "displayserver_linux.h"
#include "util/stringUtils.h"

FFDisplayType ffdsGetDisplayType(const char* name)
{
    if(ffStrStartsWith(name, "eDP-") || ffStrStartsWith(name, "LVDS-"))
        return FF_DISPLAY_TYPE_BUILTIN;
    else if(ffStrStartsWith(name, "HDMI-") ||
            ffStrStartsWith(name, "DP-") ||
            ffStrStartsWith(name, "DisplayPort-") ||
            ffStrStartsWith(name, "DVI-") ||
            ffStrStartsWith(name, "VGA-"))
        return FF_DISPLAY_TYPE_EXTERNAL;

    return FF_DISPLAY_TYPE_UNKNOWN;
}
