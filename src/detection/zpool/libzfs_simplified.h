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

typedef enum {
    ZPOOL_PROP_INVAL = -1,
    ZPOOL_PROP_NAME,
    ZPOOL_PROP_SIZE,
    ZPOOL_PROP_CAPACITY,
    ZPOOL_PROP_ALTROOT,
    ZPOOL_PROP_HEALTH,
    ZPOOL_PROP_GUID,
    ZPOOL_PROP_VERSION,
    ZPOOL_PROP_BOOTFS,
    ZPOOL_PROP_DELEGATION,
    ZPOOL_PROP_AUTOREPLACE,
    ZPOOL_PROP_CACHEFILE,
    ZPOOL_PROP_FAILUREMODE,
    ZPOOL_PROP_LISTSNAPS,
    ZPOOL_PROP_AUTOEXPAND,
    ZPOOL_PROP_DEDUPDITTO,
    ZPOOL_PROP_DEDUPRATIO,
    ZPOOL_PROP_FREE,
    ZPOOL_PROP_ALLOCATED,
    ZPOOL_PROP_READONLY,
    ZPOOL_PROP_ASHIFT,
    ZPOOL_PROP_COMMENT,
    ZPOOL_PROP_EXPANDSZ,
    ZPOOL_PROP_FREEING,
    ZPOOL_PROP_FRAGMENTATION,
    ZPOOL_PROP_LEAKED,
    ZPOOL_PROP_MAXBLOCKSIZE,
    ZPOOL_PROP_TNAME,
    ZPOOL_PROP_MAXDNODESIZE,
    ZPOOL_PROP_MULTIHOST,
    ZPOOL_PROP_CHECKPOINT,
    ZPOOL_PROP_LOAD_GUID,
    ZPOOL_PROP_AUTOTRIM,
    ZPOOL_PROP_COMPATIBILITY,
    ZPOOL_PROP_BCLONEUSED,
    ZPOOL_PROP_BCLONESAVED,
    ZPOOL_PROP_BCLONERATIO,
    ZPOOL_NUM_PROPS
} zpool_prop_t;

typedef enum {
    ZPROP_SRC_NONE = 0x1,
    ZPROP_SRC_DEFAULT = 0x2,
    ZPROP_SRC_TEMPORARY = 0x4,
    ZPROP_SRC_LOCAL = 0x8,
    ZPROP_SRC_INHERITED = 0x10,
    ZPROP_SRC_RECEIVED = 0x20
} zprop_source_t;

typedef bool boolean_t;

typedef struct libzfs_handle libzfs_handle_t;
typedef struct zpool_handle zpool_handle_t;
typedef int (*zpool_iter_f)(zpool_handle_t *, void *);
extern int zpool_iter(libzfs_handle_t *, zpool_iter_f, void *);

extern libzfs_handle_t *libzfs_init(void);
extern void libzfs_fini(libzfs_handle_t *);
extern uint64_t zpool_get_prop_int(zpool_handle_t *, zpool_prop_t, zprop_source_t *);
extern const char *zpool_get_name(zpool_handle_t *);
extern const char *zpool_get_state_str(zpool_handle_t *);
