#include "fastfetch.h"
#include "options/library.h"
#include "util/stringUtils.h"

const char* ffOptionsParseLibraryJsonConfig(FFOptionsLibrary* options, yyjson_val* root)
{
    yyjson_val* object = yyjson_obj_get(root, "library");
    if (!object) return NULL;
    if (!yyjson_is_obj(object)) return "Property 'library' must be an object";

    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(object, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);

        if (ffStrEqualsIgnCase(key, "pci"))
            ffStrbufSetS(&options->libPCI, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "vulkan"))
            ffStrbufSetS(&options->libVulkan, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "freetype"))
            ffStrbufSetS(&options->libfreetype, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "wayland"))
            ffStrbufSetS(&options->libWayland, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "xcbRandr"))
            ffStrbufSetS(&options->libXcbRandr, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "xcb"))
            ffStrbufSetS(&options->libXcb, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "Xrandr"))
            ffStrbufSetS(&options->libXrandr, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "X11"))
            ffStrbufSetS(&options->libX11, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "gio"))
            ffStrbufSetS(&options->libGIO, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "DConf"))
            ffStrbufSetS(&options->libDConf, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "dbus"))
            ffStrbufSetS(&options->libDBus, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "XFConf"))
            ffStrbufSetS(&options->libXFConf, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "sqlite") || ffStrEqualsIgnCase(key, "sqlite3"))
            ffStrbufSetS(&options->libSQLite3, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "rpm"))
            ffStrbufSetS(&options->librpm, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "imagemagick"))
            ffStrbufSetS(&options->libImageMagick, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "z"))
            ffStrbufSetS(&options->libZ, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "chafa"))
            ffStrbufSetS(&options->libChafa, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "egl"))
            ffStrbufSetS(&options->libEGL, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "glx"))
            ffStrbufSetS(&options->libGLX, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "osmesa"))
            ffStrbufSetS(&options->libOSMesa, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "opencl"))
            ffStrbufSetS(&options->libOpenCL, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "pulse"))
            ffStrbufSetS(&options->libPulse, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "nm"))
            ffStrbufSetS(&options->libnm, yyjson_get_str(val));
        else if (ffStrEqualsIgnCase(key, "ddcutil"))
            ffStrbufSetS(&options->libDdcutil, yyjson_get_str(val));
        else
            return "Unknown library property";
    }

    return NULL;
}

bool ffOptionsParseLibraryCommandLine(FFOptionsLibrary* options, const char* key, const char* value)
{
    if(ffStrStartsWithIgnCase(key, "--lib-"))
    {
        const char* subkey = key + strlen("--lib-");
        if(ffStrEqualsIgnCase(subkey, "PCI"))
            ffOptionParseString(key, value, &options->libPCI);
        else if(ffStrEqualsIgnCase(subkey, "vulkan"))
            ffOptionParseString(key, value, &options->libVulkan);
        else if(ffStrEqualsIgnCase(subkey, "freetype"))
            ffOptionParseString(key, value, &options->libfreetype);
        else if(ffStrEqualsIgnCase(subkey, "wayland"))
            ffOptionParseString(key, value, &options->libWayland);
        else if(ffStrEqualsIgnCase(subkey, "xcb-randr"))
            ffOptionParseString(key, value, &options->libXcbRandr);
        else if(ffStrEqualsIgnCase(subkey, "xcb"))
            ffOptionParseString(key, value, &options->libXcb);
        else if(ffStrEqualsIgnCase(subkey, "Xrandr"))
            ffOptionParseString(key, value, &options->libXrandr);
        else if(ffStrEqualsIgnCase(subkey, "X11"))
            ffOptionParseString(key, value, &options->libX11);
        else if(ffStrEqualsIgnCase(subkey, "gio"))
            ffOptionParseString(key, value, &options->libGIO);
        else if(ffStrEqualsIgnCase(subkey, "DConf"))
            ffOptionParseString(key, value, &options->libDConf);
        else if(ffStrEqualsIgnCase(subkey, "dbus"))
            ffOptionParseString(key, value, &options->libDBus);
        else if(ffStrEqualsIgnCase(subkey, "XFConf"))
            ffOptionParseString(key, value, &options->libXFConf);
        else if(ffStrEqualsIgnCase(subkey, "sqlite") || ffStrEqualsIgnCase(subkey, "sqlite3"))
            ffOptionParseString(key, value, &options->libSQLite3);
        else if(ffStrEqualsIgnCase(subkey, "rpm"))
            ffOptionParseString(key, value, &options->librpm);
        else if(ffStrEqualsIgnCase(subkey, "imagemagick"))
            ffOptionParseString(key, value, &options->libImageMagick);
        else if(ffStrEqualsIgnCase(subkey, "z"))
            ffOptionParseString(key, value, &options->libZ);
        else if(ffStrEqualsIgnCase(subkey, "chafa"))
            ffOptionParseString(key, value, &options->libChafa);
        else if(ffStrEqualsIgnCase(subkey, "egl"))
            ffOptionParseString(key, value, &options->libEGL);
        else if(ffStrEqualsIgnCase(subkey, "glx"))
            ffOptionParseString(key, value, &options->libGLX);
        else if(ffStrEqualsIgnCase(subkey, "osmesa"))
            ffOptionParseString(key, value, &options->libOSMesa);
        else if(ffStrEqualsIgnCase(subkey, "opencl"))
            ffOptionParseString(key, value, &options->libOpenCL);
        else if(ffStrEqualsIgnCase(subkey, "pulse"))
            ffOptionParseString(key, value, &options->libPulse);
        else if(ffStrEqualsIgnCase(subkey, "nm"))
            ffOptionParseString(key, value, &options->libnm);
        else if(ffStrEqualsIgnCase(subkey, "ddcutil"))
            ffOptionParseString(key, value, &options->libDdcutil);
        else
            return false;
    }
    else
        return false;

    return true;
}

void ffOptionsInitLibrary(FFOptionsLibrary* options)
{
    ffStrbufInit(&options->libPCI);
    ffStrbufInit(&options->libVulkan);
    ffStrbufInit(&options->libWayland);
    ffStrbufInit(&options->libXcbRandr);
    ffStrbufInit(&options->libXcb);
    ffStrbufInit(&options->libXrandr);
    ffStrbufInit(&options->libX11);
    ffStrbufInit(&options->libGIO);
    ffStrbufInit(&options->libDConf);
    ffStrbufInit(&options->libDBus);
    ffStrbufInit(&options->libXFConf);
    ffStrbufInit(&options->libSQLite3);
    ffStrbufInit(&options->librpm);
    ffStrbufInit(&options->libImageMagick);
    ffStrbufInit(&options->libZ);
    ffStrbufInit(&options->libChafa);
    ffStrbufInit(&options->libEGL);
    ffStrbufInit(&options->libGLX);
    ffStrbufInit(&options->libOSMesa);
    ffStrbufInit(&options->libOpenCL);
    ffStrbufInit(&options->libfreetype);
    ffStrbufInit(&options->libPulse);
    ffStrbufInit(&options->libnm);
    ffStrbufInit(&options->libDdcutil);
}

void ffOptionsDestroyLibrary(FFOptionsLibrary* options)
{
    ffStrbufDestroy(&options->libPCI);
    ffStrbufDestroy(&options->libVulkan);
    ffStrbufDestroy(&options->libWayland);
    ffStrbufDestroy(&options->libXcbRandr);
    ffStrbufDestroy(&options->libXcb);
    ffStrbufDestroy(&options->libXrandr);
    ffStrbufDestroy(&options->libX11);
    ffStrbufDestroy(&options->libGIO);
    ffStrbufDestroy(&options->libDConf);
    ffStrbufDestroy(&options->libDBus);
    ffStrbufDestroy(&options->libXFConf);
    ffStrbufDestroy(&options->libSQLite3);
    ffStrbufDestroy(&options->librpm);
    ffStrbufDestroy(&options->libImageMagick);
    ffStrbufDestroy(&options->libZ);
    ffStrbufDestroy(&options->libChafa);
    ffStrbufDestroy(&options->libEGL);
    ffStrbufDestroy(&options->libGLX);
    ffStrbufDestroy(&options->libOSMesa);
    ffStrbufDestroy(&options->libOpenCL);
    ffStrbufDestroy(&options->libfreetype);
    ffStrbufDestroy(&options->libPulse);
    ffStrbufDestroy(&options->libnm);
    ffStrbufDestroy(&options->libDdcutil);
}
