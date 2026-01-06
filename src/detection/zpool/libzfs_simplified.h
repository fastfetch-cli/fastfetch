#pragma once

#include "fastfetch.h"

// From https://github.com/openzfs/zfs/blob/master/include/libzfs.h

/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or https://opensource.org/licenses/CDDL-1.0.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

// zpool_prop_t and zprop_source_t were previously enums in upstream OpenZFS.
// However, the enum values for these types vary greatly between different platforms,
// making it unsafe to use the enum values directly. To ensure portability,
// we define them as simple int typedefs and use zpool_name_to_prop to look up
// the correct value for a property at runtime.
typedef int zpool_prop_t;
typedef int zprop_source_t;
typedef int boolean_t;

typedef struct libzfs_handle libzfs_handle_t;
typedef struct zpool_handle zpool_handle_t;
typedef int (*zpool_iter_f)(zpool_handle_t *, void *);

extern libzfs_handle_t *libzfs_init(void);
extern void libzfs_fini(libzfs_handle_t *);
extern int zpool_iter(libzfs_handle_t *, zpool_iter_f, void *);
extern zpool_prop_t zpool_name_to_prop(const char *);
// https://github.com/openzfs/zfs/blob/06c73cffabc30b61a695988ec8e290f43cb3768d/lib/libzfs/libzfs_pool.c#L300
extern uint64_t zpool_get_prop_int(zpool_handle_t *zhp, zpool_prop_t prop, zprop_source_t *srctype);
extern int zpool_get_prop(zpool_handle_t *zhp, zpool_prop_t prop, char *buf, size_t len, zprop_source_t *srctype, boolean_t literal);
extern void zpool_close(zpool_handle_t *);
