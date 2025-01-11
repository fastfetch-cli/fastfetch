#include "wayland.h"
#include "xx-color-management-v4-client-protocol.h"

static void waylandSupportedPrimariesNamed(void* data, FF_MAYBE_UNUSED struct xx_color_manager_v4* manager, uint32_t primaries)
{
    WaylandData* wldata = data;
    switch (primaries)
    {
        case XX_COLOR_MANAGER_V4_PRIMARIES_SRGB:
            printf("Supported named primaries: SRGB\n");
            break;
        case XX_COLOR_MANAGER_V4_PRIMARIES_PAL_M:
            printf("Supported named primaries: PAL-M\n");
            break;
        case XX_COLOR_MANAGER_V4_PRIMARIES_PAL:
            printf("Supported named primaries: PAL\n");
            break;
        case XX_COLOR_MANAGER_V4_PRIMARIES_NTSC:
            printf("Supported named primaries: NTSC\n");
            break;
        case XX_COLOR_MANAGER_V4_PRIMARIES_GENERIC_FILM:
            printf("Supported named primaries: Generic Film\n");
            break;
        case XX_COLOR_MANAGER_V4_PRIMARIES_BT2020:
            printf("Supported named primaries: BT2020\n");
            break;
        case XX_COLOR_MANAGER_V4_PRIMARIES_CIE1931_XYZ:
            printf("Supported named primaries: CIE1931 XYZ\n");
            break;
        case XX_COLOR_MANAGER_V4_PRIMARIES_DCI_P3:
            printf("Supported named primaries: DCI-P3\n");
            break;
        case XX_COLOR_MANAGER_V4_PRIMARIES_DISPLAY_P3:
            printf("Supported named primaries: Display P3\n");
            break;
        case XX_COLOR_MANAGER_V4_PRIMARIES_ADOBE_RGB:
            printf("Supported named primaries: Adobe RGB\n");
            break;
        default:
            printf("Supported named primaries: Unknown\n");
            break;
    }
}

static void waylandSupportedTfNamed(void* data, FF_MAYBE_UNUSED struct xx_color_manager_v4* manager, uint32_t tf)
{
    WaylandData* wldata = data;
    switch (tf)
    {
        case XX_COLOR_MANAGER_V4_TRANSFER_FUNCTION_BT709:
            printf("Supported named transfer function: BT709\n");
            break;
        case XX_COLOR_MANAGER_V4_TRANSFER_FUNCTION_GAMMA22:
            printf("Supported named transfer function: Gamma 2.2\n");
            break;
        case XX_COLOR_MANAGER_V4_TRANSFER_FUNCTION_GAMMA28:
            printf("Supported named transfer function: Gamma 2.8\n");
            break;
        case XX_COLOR_MANAGER_V4_TRANSFER_FUNCTION_ST240:
            printf("Supported named transfer function: ST240\n");
            break;
        case XX_COLOR_MANAGER_V4_TRANSFER_FUNCTION_LINEAR:
            printf("Supported named transfer function: Linear\n");
            break;
        case XX_COLOR_MANAGER_V4_TRANSFER_FUNCTION_LOG_100:
            printf("Supported named transfer function: Log 100\n");
            break;
        case XX_COLOR_MANAGER_V4_TRANSFER_FUNCTION_LOG_316:
            printf("Supported named transfer function: Log 316\n");
            break;
        case XX_COLOR_MANAGER_V4_TRANSFER_FUNCTION_XVYCC:
            printf("Supported named transfer function: XVYCC\n");
            break;
        case XX_COLOR_MANAGER_V4_TRANSFER_FUNCTION_BT1361:
            printf("Supported named transfer function: BT1361\n");
            break;
        case XX_COLOR_MANAGER_V4_TRANSFER_FUNCTION_SRGB:
            printf("Supported named transfer function: SRGB\n");
            break;
        case XX_COLOR_MANAGER_V4_TRANSFER_FUNCTION_EXT_SRGB:
            printf("Supported named transfer function: Extended SRGB\n");
            break;
        case XX_COLOR_MANAGER_V4_TRANSFER_FUNCTION_ST2084_PQ:
            printf("Supported named transfer function: ST2084 PQ\n");
            break;
        case XX_COLOR_MANAGER_V4_TRANSFER_FUNCTION_ST428:
            printf("Supported named transfer function: ST428\n");
            break;
        case XX_COLOR_MANAGER_V4_TRANSFER_FUNCTION_HLG:
            printf("Supported named transfer function: HLG\n");
            break;
        default:
            printf("Supported named transfer function: Unknown\n");
            break;
    }
}

void ffWaylandHandleColorManager(WaylandData* wldata, struct wl_registry* registry, uint32_t name, uint32_t version)
{
    struct wl_proxy* output = wldata->ffwl_proxy_marshal_constructor_versioned((struct wl_proxy*) registry, WL_REGISTRY_BIND, &xx_color_manager_v4_interface, version, name, xx_color_manager_v4_interface.name, version, NULL);
    if(output == NULL)
        return;

    struct xx_color_manager_v4_listener managerListener = {
        .supported_feature = (void*) stubListener,
        .supported_intent = (void*) stubListener,
        .supported_primaries_named = waylandSupportedPrimariesNamed,
        .supported_tf_named = waylandSupportedTfNamed,
    };

    wldata->ffwl_proxy_add_listener(output, (void(**)(void)) &managerListener, wldata);
    wldata->ffwl_display_roundtrip(wldata->display);
    wldata->ffwl_proxy_destroy(output);
}
