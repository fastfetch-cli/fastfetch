#include "camera.h"

#import <AVFoundation/AVCaptureDevice.h>

const char* ffDetectCamera(FFlist* result)
{
    AVCaptureDeviceDiscoverySession* session = [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:@[AVCaptureDeviceTypeBuiltInWideAngleCamera] mediaType:AVMediaTypeVideo position:AVCaptureDevicePositionUnspecified];
    if (!session)
        return "Failed to create AVCaptureDeviceDiscoverySession";

    for (AVCaptureDevice* device in session.devices)
    {
        FFCameraResult* camera = (FFCameraResult*) ffListAdd(result);
        ffStrbufInitS(&camera->name, device.localizedName.UTF8String);
        ffStrbufInitS(&camera->vendor, device.manufacturer.UTF8String);
        ffStrbufInitS(&camera->id, device.uniqueID.UTF8String);

        CMVideoDimensions size = CMVideoFormatDescriptionGetDimensions(device.activeFormat.formatDescription);
        camera->width = size.width < 0 ? 0 : (uint32_t) size.width;
        camera->height = size.height < 0 ? 0 : (uint32_t) size.height;
    }

    return NULL;
}
