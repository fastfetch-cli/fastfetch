#include "displayserver_linux.h"
#include "util/stringUtils.h"
#include "common/processing.h"

static inline void wrapYyjsonFree(yyjson_doc** doc)
{
    assert(doc);
    if (*doc)
        yyjson_doc_free(*doc);
}

const char* ffdsConnectWlroots(FFDisplayServerResult* result)
{
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    if (ffProcessAppendStdOut(&buffer, (char* const[]){
        "wlr-randr",
        "--json",
        NULL
    }) != NULL || buffer.length == 0)
        return "Running wlr-randr failed";

    yyjson_doc* __attribute__((__cleanup__(wrapYyjsonFree))) doc = yyjson_read_opts(buffer.chars, buffer.length, 0, NULL, NULL);
    if (!doc)
        return "Failed to parse wlr-randr info";

    yyjson_val* root = yyjson_doc_get_root(doc);
    if (!yyjson_is_arr(root))
        return "Battery info result is not a JSON array";

    yyjson_val* device;
    size_t idev, mdev;
    yyjson_arr_foreach(root, idev, mdev, device)
    {
        yyjson_val* modes = yyjson_obj_get(device, "modes");
        if (!yyjson_is_arr(modes))
            continue;

        if (!yyjson_get_bool(yyjson_obj_get(device, "enabled")))
            continue;

        yyjson_val* mode;
        size_t imode, mmode;
        yyjson_arr_foreach(modes, imode, mmode, mode)
        {
            if (!yyjson_is_obj(mode))
                continue;

            if (!yyjson_get_bool(yyjson_obj_get(mode, "current")))
                continue;

            uint32_t width = (uint32_t) yyjson_get_uint(yyjson_obj_get(mode, "width"));
            uint32_t height = (uint32_t) yyjson_get_uint(yyjson_obj_get(mode, "height"));

            if (width == 0 || height == 0)
                continue;

            double refreshRate = yyjson_get_real(yyjson_obj_get(mode, "refresh"));

            double scale = (double) yyjson_get_real(yyjson_obj_get(device, "scale"));
            const char* connName = yyjson_get_str(yyjson_obj_get(device, "name"));
            FFDisplayType type = FF_DISPLAY_TYPE_UNKNOWN;
            if(ffStrStartsWith(connName, "eDP-"))
                type = FF_DISPLAY_TYPE_BUILTIN;
            else if(ffStrStartsWith(connName, "HDMI-"))
                type = FF_DISPLAY_TYPE_EXTERNAL;

            FF_STRBUF_AUTO_DESTROY displayName = ffStrbufCreate();
            if (!ffdsMatchDrmConnector(connName, &displayName))
                ffStrbufSetS(&displayName, yyjson_get_str(yyjson_obj_get(device, "description")));
            uint32_t rotation = 0;
            const char* transform = yyjson_get_str(yyjson_obj_get(device, "transform"));
            if (!ffStrEquals(transform, "normal"))
            {
                // 90 | flipped-90
                if (ffStrEndsWith(transform, "90"))
                    rotation = 90;
                else if (ffStrEndsWith(transform, "180"))
                    rotation = 180;
                else if (ffStrEndsWith(transform, "270"))
                    rotation = 270;
            }
            if (rotation % 180 != 0)
            {
                uint32_t temp = width;
                width = height;
                height = temp;
            }
            ffdsAppendDisplay(result,
                width,
                height,
                refreshRate,
                (uint32_t) (width / scale),
                (uint32_t) (height / scale),
                rotation,
                &displayName,
                type,
                false,
                0
            );
            break;
        }
    }

    return NULL;
}
