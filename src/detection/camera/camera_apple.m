#include "camera.h"
#include "common/io/io.h"

#import <AVFoundation/AVCaptureDevice.h>

const char* ffDetectCamera(FFlist* result)
{
    FF_SUPPRESS_IO(); // #822

    AVCaptureDeviceDiscoverySession* session = NULL;
    if (@available(macOS 10.15, *)) {
        session =  [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:@[AVCaptureDeviceTypeBuiltInWideAngleCamera, AVCaptureDeviceTypeExternal]
                                                                          mediaType:AVMediaTypeVideo
                                                                           position:AVCaptureDevicePositionUnspecified];

    } else {
        session =  [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:@[AVCaptureDeviceTypeBuiltInWideAngleCamera, AVCaptureDeviceTypeExternalUnknown]
                                                                          mediaType:AVMediaTypeVideo
                                                                           position:AVCaptureDevicePositionUnspecified];
    }

    if (!session)
        return "Failed to create AVCaptureDeviceDiscoverySession";

    for (AVCaptureDevice* device in session.devices)
    {
        FFCameraResult* camera = (FFCameraResult*) ffListAdd(result);
        ffStrbufInitS(&camera->name, device.localizedName.UTF8String);
        ffStrbufInitS(&camera->vendor, device.manufacturer.UTF8String);
        ffStrbufInitS(&camera->id, device.uniqueID.UTF8String);
        switch (device.activeColorSpace)
        {
            case AVCaptureColorSpace_sRGB: ffStrbufInitStatic(&camera->colorspace, "sRGB"); break;
            case AVCaptureColorSpace_P3_D65: ffStrbufInitStatic(&camera->colorspace, "P3-D65"); break;
            case 2 /*AVCaptureColorSpace_HLG_BT2020*/: ffStrbufInitStatic(&camera->colorspace, "BT2020-HLG"); break;
            case 3 /*AVCaptureColorSpace_AppleLog*/: ffStrbufInitStatic(&camera->colorspace, "AppleLog"); break;
        }

        CMVideoDimensions size = CMVideoFormatDescriptionGetDimensions(device.activeFormat.formatDescription);
        camera->width = size.width < 0 ? 0 : (uint32_t) size.width;
        camera->height = size.height < 0 ? 0 : (uint32_t) size.height;
    }

    return NULL;
}
