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

typedef enum zpool_errata {
    ZPOOL_ERRATA_NONE,
    ZPOOL_ERRATA_ZOL_2094_SCRUB,
    ZPOOL_ERRATA_ZOL_2094_ASYNC_DESTROY,
    ZPOOL_ERRATA_ZOL_6845_ENCRYPTION,
    ZPOOL_ERRATA_ZOL_8308_ENCRYPTION,
} zpool_errata_t;

typedef enum {
    /*
     * The following correspond to faults as defined in the (fault.fs.zfs.*)
     * event namespace.  Each is associated with a corresponding message ID.
     * This must be kept in sync with the zfs_msgid_table in
     * lib/libzfs/libzfs_status.c.
     */
    ZPOOL_STATUS_CORRUPT_CACHE,	/* corrupt /kernel/drv/zpool.cache */
    ZPOOL_STATUS_MISSING_DEV_R,	/* missing device with replicas */
    ZPOOL_STATUS_MISSING_DEV_NR,	/* missing device with no replicas */
    ZPOOL_STATUS_CORRUPT_LABEL_R,	/* bad device label with replicas */
    ZPOOL_STATUS_CORRUPT_LABEL_NR,	/* bad device label with no replicas */
    ZPOOL_STATUS_BAD_GUID_SUM,	/* sum of device guids didn't match */
    ZPOOL_STATUS_CORRUPT_POOL,	/* pool metadata is corrupted */
    ZPOOL_STATUS_CORRUPT_DATA,	/* data errors in user (meta)data */
    ZPOOL_STATUS_FAILING_DEV,	/* device experiencing errors */
    ZPOOL_STATUS_VERSION_NEWER,	/* newer on-disk version */
    ZPOOL_STATUS_HOSTID_MISMATCH,	/* last accessed by another system */
    ZPOOL_STATUS_HOSTID_ACTIVE,	/* currently active on another system */
    ZPOOL_STATUS_HOSTID_REQUIRED,	/* multihost=on and hostid=0 */
    ZPOOL_STATUS_IO_FAILURE_WAIT,	/* failed I/O, failmode 'wait' */
    ZPOOL_STATUS_IO_FAILURE_CONTINUE, /* failed I/O, failmode 'continue' */
    ZPOOL_STATUS_IO_FAILURE_MMP,	/* failed MMP, failmode not 'panic' */
    ZPOOL_STATUS_BAD_LOG,		/* cannot read log chain(s) */
    ZPOOL_STATUS_ERRATA,		/* informational errata available */

    /*
     * If the pool has unsupported features but can still be opened in
     * read-only mode, its status is ZPOOL_STATUS_UNSUP_FEAT_WRITE. If the
     * pool has unsupported features but cannot be opened at all, its
     * status is ZPOOL_STATUS_UNSUP_FEAT_READ.
     */
    ZPOOL_STATUS_UNSUP_FEAT_READ,	/* unsupported features for read */
    ZPOOL_STATUS_UNSUP_FEAT_WRITE,	/* unsupported features for write */

    /*
     * These faults have no corresponding message ID.  At the time we are
     * checking the status, the original reason for the FMA fault (I/O or
     * checksum errors) has been lost.
     */
    ZPOOL_STATUS_FAULTED_DEV_R,	/* faulted device with replicas */
    ZPOOL_STATUS_FAULTED_DEV_NR,	/* faulted device with no replicas */

    /*
     * The following are not faults per se, but still an error possibly
     * requiring administrative attention.  There is no corresponding
     * message ID.
     */
    ZPOOL_STATUS_VERSION_OLDER,	/* older legacy on-disk version */
    ZPOOL_STATUS_FEAT_DISABLED,	/* supported features are disabled */
    ZPOOL_STATUS_RESILVERING,	/* device being resilvered */
    ZPOOL_STATUS_OFFLINE_DEV,	/* device offline */
    ZPOOL_STATUS_REMOVED_DEV,	/* removed device */
    ZPOOL_STATUS_REBUILDING,	/* device being rebuilt */
    ZPOOL_STATUS_REBUILD_SCRUB,	/* recommend scrubbing the pool */
    ZPOOL_STATUS_NON_NATIVE_ASHIFT,	/* (e.g. 512e dev with ashift of 9) */
    ZPOOL_STATUS_COMPATIBILITY_ERR,	/* bad 'compatibility' property */
    ZPOOL_STATUS_INCOMPATIBLE_FEAT,	/* feature set outside compatibility */

    /*
     * Finally, the following indicates a healthy pool.
     */
    ZPOOL_STATUS_OK
} zpool_status_t;

#ifndef __sun
typedef bool boolean_t;
#endif

typedef struct libzfs_handle libzfs_handle_t;
typedef struct zpool_handle zpool_handle_t;
typedef int (*zpool_iter_f)(zpool_handle_t *, void *);
extern int zpool_iter(libzfs_handle_t *, zpool_iter_f, void *);

extern libzfs_handle_t *libzfs_init(void);
extern void libzfs_fini(libzfs_handle_t *);
extern uint64_t zpool_get_prop_int(zpool_handle_t *, zpool_prop_t, zprop_source_t *);
extern const char *zpool_get_name(zpool_handle_t *);
extern const char *zpool_get_state_str(zpool_handle_t *);
extern zpool_status_t zpool_get_status(zpool_handle_t *, const char **, zpool_errata_t *);
