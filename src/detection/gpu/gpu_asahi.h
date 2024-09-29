#if __aarch64__ && FF_HAVE_DRM

#include <drm.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#if __has_include(<drm/asahi_drm.h>)

#include <drm/asahi_drm.h>

#else

// https://github.com/AsahiLinux/linux/blob/asahi/include/uapi/drm/asahi_drm.h
/* SPDX-License-Identifier: MIT */
/* Copyright (C) The Asahi Linux Contributors */

#define DRM_ASAHI_UNSTABLE_UABI_VERSION		10011

#define DRM_ASAHI_GET_PARAMS			0x00
#define DRM_ASAHI_MAX_CLUSTERS	32
struct drm_asahi_params_global
{
    __u32 unstable_uabi_version;
    __u32 pad0;

    __u64 feat_compat;
    __u64 feat_incompat;

    __u32 gpu_generation;
    __u32 gpu_variant;
    __u32 gpu_revision;
    __u32 chip_id;

    __u32 num_dies;
    __u32 num_clusters_total;
    __u32 num_cores_per_cluster;
    __u32 num_frags_per_cluster;
    __u32 num_gps_per_cluster;
    __u32 num_cores_total_active;
    __u64 core_masks[DRM_ASAHI_MAX_CLUSTERS];

    __u32 vm_page_size;
    __u32 pad1;
    __u64 vm_user_start;
    __u64 vm_user_end;
    __u64 vm_usc_start;
    __u64 vm_usc_end;
    __u64 vm_kernel_min_size;

    __u32 max_syncs_per_submission;
    __u32 max_commands_per_submission;
    __u32 max_commands_in_flight;
    __u32 max_attachments;

    __u32 timer_frequency_hz;
    __u32 min_frequency_khz;
    __u32 max_frequency_khz;
    __u32 max_power_mw;

    __u32 result_render_size;
    __u32 result_compute_size;

    __u32 firmware_version[4];
};

struct drm_asahi_get_params
{
    /** @extensions: Pointer to the first extension struct, if any */
    __u64 extensions;

    /** @param: Parameter group to fetch (MBZ) */
    __u32 param_group;

    /** @pad: MBZ */
    __u32 pad;

    /** @value: User pointer to write parameter struct */
    __u64 pointer;

    /** @value: Size of user buffer, max size supported on return */
    __u64 size;
};

enum
{
    DRM_IOCTL_ASAHI_GET_PARAMS       = DRM_IOWR(DRM_COMMAND_BASE + DRM_ASAHI_GET_PARAMS, struct drm_asahi_get_params),
};

#endif // __has_include

#endif // FF_HAVE_DRM
