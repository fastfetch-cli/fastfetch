#include "mouse.h"
#include "common/io/io.h"

#include <stdio.h>
#include <fcntl.h>
#include <usbhid.h>
#include <dev/usb/usb_ioctl.h>

#define MAX_UHID_JOYS 64

const char* ffDetectMouse(FFlist* devices /* List of FFMouseDevice */)
{
    char path[16];
    for (int i = 0; i < MAX_UHID_JOYS; i++)
    {
        snprintf(path, ARRAY_SIZE(path), "/dev/uhid%d", i);
        FF_AUTO_CLOSE_FD int fd = open(path, O_RDONLY | O_CLOEXEC);
        if (fd < 0) continue;

        report_desc_t repDesc = hid_get_report_desc(fd);
        if (!repDesc) continue;

        int reportId = hid_get_report_id(fd);

        struct hid_data* hData = hid_start_parse(repDesc, 0, reportId);
        if (hData)
        {
            struct hid_item hItem;
            while (hid_get_item(hData, &hItem) > 0)
            {
                if (HID_PAGE(hItem.usage) != 1 || HID_USAGE(hItem.usage) != 2) continue;

                struct usb_device_info di;
                if (ioctl(fd, USB_GET_DEVICEINFO, &di) != -1)
                {
                    FFMouseDevice* device = (FFMouseDevice*) ffListAdd(devices);
                    ffStrbufInitS(&device->serial, di.udi_serial);
                    ffStrbufInitS(&device->name, di.udi_product);
                }
            }
        }

        hid_dispose_report_desc(repDesc);
    }

    return NULL;
}
