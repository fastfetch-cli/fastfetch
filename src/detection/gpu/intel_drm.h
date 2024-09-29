#pragma once

/* SPDX-License-Identifier: MIT */
#include <drm.h>

// xe_drm.h

/*
 * Copyright © 2023 Intel Corporation
 */

#define DRM_XE_DEVICE_QUERY		0x00

#define DRM_IOCTL_XE_DEVICE_QUERY		DRM_IOWR(DRM_COMMAND_BASE + DRM_XE_DEVICE_QUERY, struct drm_xe_device_query)

enum drm_xe_memory_class {
	DRM_XE_MEM_REGION_CLASS_SYSMEM = 0,
	DRM_XE_MEM_REGION_CLASS_VRAM
};

struct drm_xe_mem_region {
	__u16 mem_class;
	__u16 instance;
	__u32 min_page_size;
	__u64 total_size;
	__u64 used;
	__u64 cpu_visible_size;
	__u64 cpu_visible_used;
	__u64 reserved[6];
};

struct drm_xe_query_mem_regions {
	__u32 num_mem_regions;
	__u32 pad;
	struct drm_xe_mem_region mem_regions[];
};

struct drm_xe_query_topology_mask {
	__u16 gt_id;

#define DRM_XE_TOPO_DSS_GEOMETRY	1
#define DRM_XE_TOPO_DSS_COMPUTE		2
#define DRM_XE_TOPO_EU_PER_DSS		4
	__u16 type;
	__u32 num_bytes;
	__u8 mask[];
};

struct drm_xe_device_query {
	__u64 extensions;

#define DRM_XE_DEVICE_QUERY_MEM_REGIONS		1
#define DRM_XE_DEVICE_QUERY_GT_TOPOLOGY		5
	__u32 query;
	__u32 size;
	__u64 data;
	__u64 reserved[2];
};

// i915_drm.h

/*
 * Copyright 2003 Tungsten Graphics, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 */

#define DRM_IOCTL_I915_GETPARAM         DRM_IOWR(DRM_COMMAND_BASE + DRM_I915_GETPARAM, drm_i915_getparam_t)

struct drm_i915_getparam {
	__s32 param;
	int *value;
};
typedef struct drm_i915_getparam drm_i915_getparam_t;

#define DRM_I915_GETPARAM	0x06
#define DRM_I915_QUERY			0x39
#define DRM_I915_QUERY_MEMORY_REGIONS		4
#define DRM_IOCTL_I915_QUERY			DRM_IOWR(DRM_COMMAND_BASE + DRM_I915_QUERY, struct drm_i915_query)
#define I915_PARAM_EU_TOTAL		 34

struct drm_i915_query_item {
	__u64 query_id;
#define DRM_I915_QUERY_MEMORY_REGIONS		4

	__s32 length;
	__u32 flags;
	__u64 data_ptr;
};

struct drm_i915_query {
	__u32 num_items;
	__u32 flags;
	__u64 items_ptr;
};

enum drm_i915_gem_memory_class {
	I915_MEMORY_CLASS_SYSTEM = 0,
	I915_MEMORY_CLASS_DEVICE,
};

struct drm_i915_gem_memory_class_instance {
	__u16 memory_class;
	__u16 memory_instance;
};

struct drm_i915_memory_region_info {
	struct drm_i915_gem_memory_class_instance region;
	__u32 rsvd0;
	__u64 probed_size;
	__u64 unallocated_size;

	union {
		__u64 rsvd1[8];
		struct {
			__u64 probed_cpu_visible_size;
			__u64 unallocated_cpu_visible_size;
		};
	};
};

struct drm_i915_query_memory_regions {
	__u32 num_regions;
	__u32 rsvd[3];
	struct drm_i915_memory_region_info regions[];
};
