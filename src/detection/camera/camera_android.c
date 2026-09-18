#include "camera.h"

#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraMetadata.h>
#include <camera/NdkCameraMetadataTags.h>

// HAL_PIXEL_FORMAT_JPEG, the still capture format the stream configurations are keyed by. It is
// defined in <system/graphics.h>, which the NDK does not ship, and shares its value with
// AHardwareBuffer's BLOB format.
#define FF_ANDROID_PIXEL_FORMAT_JPEG 0x21

// Records the largest size the camera can stream in the given pixel format. The stream configurations
// are a flat array of (format, width, height, isInput) quads; a negative format matches any of them.
static void ffCameraMaxStreamSize(const ACameraMetadata* metadata, int32_t format, uint32_t* width, uint32_t* height) {
    ACameraMetadata_const_entry entry;
    if (ACameraMetadata_getConstEntry(metadata, ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entry) != ACAMERA_OK) {
        return;
    }
    if (entry.type != ACAMERA_TYPE_INT32) {
        return;
    }

    uint64_t maxArea = 0;
    for (uint32_t i = 0; i + 3 < entry.count; i += 4) {
        if (entry.data.i32[i + 3] != ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT) {
            continue;
        }
        if (format >= 0 && entry.data.i32[i] != format) {
            continue;
        }

        uint32_t w = (uint32_t) entry.data.i32[i + 1];
        uint32_t h = (uint32_t) entry.data.i32[i + 2];
        if ((uint64_t) w * h > maxArea) {
            maxArea = (uint64_t) w * h;
            *width = w;
            *height = h;
        }
    }
}

const char* ffDetectCamera(FFlist* result) {
    // The camera2 NDK and every entry point below were introduced in API 24, which is the API level
    // this build targets, so nothing here is newer than the minimum supported version and
    // common/androidApi.h has nothing to guard. That header covers the opposite case: entry points
    // the NDK marks unavailable because they postdate the target, such as AImageDecoder (30) or
    // AMediaCodec_getName (28). It also only ever makes *symbols* weak -- libcamera2ndk.so itself is
    // API 24, so linking it unconditionally in CMakeLists.txt is fine on every supported device.
    ACameraManager* manager = ACameraManager_create();
    if (!manager) {
        return "ACameraManager_create() failed";
    }

    ACameraIdList* idList = nullptr;
    if (ACameraManager_getCameraIdList(manager, &idList) != ACAMERA_OK || !idList) {
        ACameraManager_delete(manager);
        return "ACameraManager_getCameraIdList() failed";
    }

    for (int32_t i = 0; i < idList->numCameras; ++i) {
        const char* id = idList->cameraIds[i];

        ACameraMetadata* metadata = nullptr;
        if (ACameraManager_getCameraCharacteristics(manager, id, &metadata) != ACAMERA_OK || !metadata) {
            continue;
        }

        FFCameraResult* camera = FF_LIST_ADD(FFCameraResult, *result);
        ffStrbufInit(&camera->vendor);
        ffStrbufInit(&camera->colorspace);
        ffStrbufInitS(&camera->id, id);

        ACameraMetadata_const_entry facing;
        if (ACameraMetadata_getConstEntry(metadata, ACAMERA_LENS_FACING, &facing) == ACAMERA_OK && facing.type == ACAMERA_TYPE_BYTE && facing.count >= 1) {
            switch (facing.data.u8[0]) {
                case ACAMERA_LENS_FACING_FRONT:
                    ffStrbufInitStatic(&camera->name, "builtin-front");
                    break;
                case ACAMERA_LENS_FACING_BACK:
                    ffStrbufInitStatic(&camera->name, "builtin-back");
                    break;
                case ACAMERA_LENS_FACING_EXTERNAL:
                    ffStrbufInitStatic(&camera->name, "builtin-external");
                    break;
                default:
                    ffStrbufInitStatic(&camera->name, "Unknown");
                    break;
            }
        } else {
            ffStrbufInitStatic(&camera->name, "Unknown");
        }

        camera->width = camera->height = 0;
        ffCameraMaxStreamSize(metadata, FF_ANDROID_PIXEL_FORMAT_JPEG, &camera->width, &camera->height);
        if (camera->width == 0) {
            // Not every camera HAL lists a JPEG format. The largest size of any output format is the
            // same resolution on those devices.
            ffCameraMaxStreamSize(metadata, -1, &camera->width, &camera->height);
        }

        ACameraMetadata_free(metadata);
    }

    ACameraManager_deleteCameraIdList(idList);
    ACameraManager_delete(manager);

    return nullptr;
}
