#pragma once

#ifdef FF_HAVE_WAYLAND

#include "common/library.h"
#include "util/stringUtils.h"

#include <wayland-client.h>

#include "../displayserver_linux.h"

typedef enum __attribute__((__packed__)) WaylandProtocolType
{
    FF_WAYLAND_PROTOCOL_TYPE_NONE,
    FF_WAYLAND_PROTOCOL_TYPE_GLOBAL,
    FF_WAYLAND_PROTOCOL_TYPE_ZWLR,
    FF_WAYLAND_PROTOCOL_TYPE_KDE,
} WaylandProtocolType;

typedef struct WaylandData
{
    FFDisplayServerResult* result;
    FF_LIBRARY_SYMBOL(wl_proxy_marshal_constructor_versioned)
    FF_LIBRARY_SYMBOL(wl_proxy_add_listener)
    FF_LIBRARY_SYMBOL(wl_proxy_destroy)
    FF_LIBRARY_SYMBOL(wl_display_roundtrip)
    struct wl_display* display;
    const struct wl_interface* ffwl_output_interface;
    WaylandProtocolType protocolType;
    uint64_t primaryDisplayId;
    struct wl_proxy* zxdgOutputManager;
} WaylandData;

typedef struct WaylandDisplay
{
    WaylandData* parent;
    int32_t width;
    int32_t height;
    int32_t refreshRate;
    int32_t preferredWidth;
    int32_t preferredHeight;
    int32_t preferredRefreshRate;
    int32_t physicalWidth;
    int32_t physicalHeight;
    double scale;
    enum wl_output_transform transform;
    FFDisplayType type;
    FFstrbuf name;
    FFstrbuf description;
    FFstrbuf edidName;
    uint64_t id;
    bool hdrInfoAvailable;
    bool hdrSupported;
    bool hdrEnabled;
    uint16_t myear;
    uint16_t mweek;
    uint32_t serial;
    void* internal;
} WaylandDisplay;

inline static void stubListener(void* data, ...)
{
    (void) data;
}

inline static uint64_t ffWaylandGenerateIdFromName(const char* name)
{
    uint64_t id = 0;
    size_t len = strlen(name);
    if (len > sizeof(id))
        memcpy(&id, name + (len - sizeof(id)), sizeof(id)); // copy the last 8 bytes
    else if (len > 0)
        memcpy(&id, name, len);
    return id;
}

void ffWaylandOutputNameListener(void* data, FF_MAYBE_UNUSED void* output, const char *name);
void ffWaylandOutputDescriptionListener(void* data, FF_MAYBE_UNUSED void* output, const char* description);
// Modifies content of display. Don't call this function when calling ffdsAppendDisplay
uint32_t ffWaylandHandleRotation(WaylandDisplay* display);

const char* ffWaylandHandleGlobalOutput(WaylandData* wldata, struct wl_registry* registry, uint32_t name, uint32_t version);
const char* ffWaylandHandleZwlrOutput(WaylandData* wldata, struct wl_registry* registry, uint32_t name, uint32_t version);
const char* ffWaylandHandleKdeOutput(WaylandData* wldata, struct wl_registry* registry, uint32_t name, uint32_t version);
const char* ffWaylandHandleKdeOutputOrder(WaylandData* wldata, struct wl_registry* registry, uint32_t name, uint32_t version);
const char* ffWaylandHandleZxdgOutput(WaylandData* wldata, struct wl_registry* registry, uint32_t name, uint32_t version);

#endif
