#include "camera.h"

#include "common/processing.h"
#include "common/properties.h"

#define FF_TERMUX_API_PATH FASTFETCH_TARGET_DIR_ROOT "/libexec/termux-api"
#define FF_TERMUX_API_PARAM "CameraInfo"

static inline void wrapYyjsonFree(yyjson_doc** doc)
{
    assert(doc);
    if (*doc)
        yyjson_doc_free(*doc);
}

const char* ffDetectCamera(FF_MAYBE_UNUSED FFlist* result)
{
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();

    if(ffProcessAppendStdOut(&buffer, (char* const[]){
        FF_TERMUX_API_PATH,
        FF_TERMUX_API_PARAM,
        NULL
    }))
        return "Starting `" FF_TERMUX_API_PATH " " FF_TERMUX_API_PARAM "` failed";

    yyjson_doc* __attribute__((__cleanup__(wrapYyjsonFree))) doc = yyjson_read_opts(buffer.chars, buffer.length, 0, NULL, NULL);
    if (!doc)
        return "Failed to parse camera info";

    yyjson_val* root = yyjson_doc_get_root(doc);
    if (!yyjson_is_arr(root))
        return "Camera info result is not a JSON array";

    yyjson_val* device;
    size_t idx, max;
    yyjson_arr_foreach(root, idx, max, device)
    {
        FFCameraResult* camera = (FFCameraResult*) ffListAdd(result);
        ffStrbufInitF(&camera->name, "Camera-%s", yyjson_get_str(yyjson_obj_get(device, "facing")));
        ffStrbufInit(&camera->vendor);
        ffStrbufInitS(&camera->id, yyjson_get_str(yyjson_obj_get(device, "id")));
        yyjson_val* format = yyjson_arr_get_first(yyjson_obj_get(device, "jpeg_output_sizes"));
        if (yyjson_is_obj(format))
        {
            camera->width = (uint32_t) yyjson_get_uint(yyjson_obj_get(format, "width"));
            camera->height = (uint32_t) yyjson_get_uint(yyjson_obj_get(format, "height"));
        }
        else
            camera->width = camera->height = 0;
    }

    return NULL;
}
