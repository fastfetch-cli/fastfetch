#pragma once

#ifdef FF_HAVE_WAYLAND

#include "common/library.h"
#include "util/stringUtils.h"

#include <wayland-client.h>

#include "../displayserver_linux.h"

typedef enum WaylandProtocolType
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
} WaylandData;

typedef struct WaylandDisplay
{
    WaylandData* parent;
    int32_t width;
    int32_t height;
    int32_t refreshRate;
    double scale;
    enum wl_output_transform transform;
    FFDisplayType type;
    FFstrbuf name;
    FFstrbuf description;
    FFstrbuf edidName;
    void* internal;
} WaylandDisplay;

inline static void stubListener(void* data, ...)
{
    (void) data;
}

void ffWaylandOutputNameListener(void* data, FF_MAYBE_UNUSED void* output, const char *name);
void ffWaylandOutputDescriptionListener(void* data, FF_MAYBE_UNUSED void* output, const char* description);

void ffWaylandHandleGlobalOutput(WaylandData* wldata, struct wl_registry* registry, uint32_t name, uint32_t version);
void ffWaylandHandleZwlrOutput(WaylandData* wldata, struct wl_registry* registry, uint32_t name, uint32_t version);
void ffWaylandHandleKdeOutput(WaylandData* wldata, struct wl_registry* registry, uint32_t name, uint32_t version);

#endif
