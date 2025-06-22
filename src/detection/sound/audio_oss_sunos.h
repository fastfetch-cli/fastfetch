/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
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
/*
 * Copyright (C) 4Front Technologies 1996-2008.
 *
 * Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */


#ifndef	_SYS_AUDIO_OSS_H
#define	_SYS_AUDIO_OSS_H

#include <sys/types.h>
#include <sys/time.h>

/*
 * These are the ioctl calls for all Solaris /dev/dsp and /dev/mixer audio
 * devices.
 *
 * Note that the contents of this file include definitions which exist
 * primarily for compatibility.  Many of the defines here are not
 * actually implemented, but exist solely to facilitate compilation of
 * programs from other operating systems.  Other definitions here may
 * not be fully supported or may otherwise be obsolete. There are many
 * things in this file which should not be used on SunOS.
 *
 * Please read the documentation to determine which portions of the
 * API are fully supported and recommended for use in new
 * applications.
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Buffer status queries.
 * SNDCTL_DSP_GETOSPACE and SNDCTL_DSP_GETISPACE
 */
typedef struct audio_buf_info {
	int fragments;		/* # of available fragments */
	int fragstotal;		/* Total # of fragments allocated */
	int fragsize;		/* Size of a fragment in bytes */
	int bytes;		/* Available space in bytes */
	/* Note! 'bytes' could be more than fragments*fragsize */
} audio_buf_info;

/*
 * Sync groups for audio devices.
 * SNDCTL_DSP_SYNCGROUP and SNDCTL_DSP_SYNCSTART
 */
typedef struct oss_syncgroup {
	int id;
	int mode;
	int filler[16];
} oss_syncgroup;

/*
 * SNDCTL_DSP_GETERROR
 */
typedef struct audio_errinfo {
	int play_underruns;
	int rec_overruns;
	unsigned int play_ptradjust;
	unsigned int rec_ptradjust;
	int play_errorcount;
	int rec_errorcount;
	int play_lasterror;
	int rec_lasterror;
	int play_errorparm;
	int rec_errorparm;
	int filler[16];
} audio_errinfo;

/*
 * SNDCTL_DSP_GETIPTR and SNDCTL_DSP_GETOPTR
 */
typedef struct count_info {
	unsigned int bytes;	/* Total # of bytes processed */
	int blocks;		/* # of fragment transitions since last time */
	int ptr;		/* Current DMA pointer value */
} count_info;

/*
 * SNDCTL_DSP_CURENT_IPTR and SNDCTL_DSP_CURRENT_OPTR
 */
typedef struct {
	long long samples;	/* Total # of samples */
	int fifo_samples;	/* Samples in device FIFO */
	int filler[32];		/* For future use */
} oss_count_t;

/*
 * SNDCTL_DSP_GET_RECSRC_NAMES and SNDCTL_DSP_GET_PLAYTGT_NAMES
 */
#define	OSS_ENUM_MAXVALUE	255
typedef struct oss_mixer_enuminfo {
	int dev;
	int ctrl;
	int nvalues;
	int version;
	short strindex[OSS_ENUM_MAXVALUE];
	char strings[3000];
} oss_mixer_enuminfo;

/*
 * Digital interface (S/PDIF) control interface
 * SNDCTL_DSP_READCTL and SNDCTL_DSP_WRITECTL
 */
typedef struct oss_digital_control {
	unsigned int caps;
#define	DIG_CBITIN_NONE		0x00000000
#define	DIG_CBITIN_LIMITED	0x00000001
#define	DIG_CBITIN_DATA		0x00000002
#define	DIG_CBITIN_BYTE0	0x00000004
#define	DIG_CBITIN_FULL		0x00000008
#define	DIG_CBITIN_MASK		0x0000000f
#define	DIG_CBITOUT_NONE	0x00000000
#define	DIG_CBITOUT_LIMITED	0x00000010
#define	DIG_CBITOUT_BYTE0	0x00000020
#define	DIG_CBITOUT_FULL	0x00000040
#define	DIG_CBITOUT_DATA	0x00000080
#define	DIG_CBITOUT_MASK	0x000000f0
#define	DIG_UBITIN		0x00000100
#define	DIG_UBITOUT		0x00000200
#define	DIG_VBITOUT		0x00000400
#define	DIG_OUTRATE		0x00000800
#define	DIG_INRATE		0x00001000
#define	DIG_INBITS		0x00002000
#define	DIG_OUTBITS		0x00004000
#define	DIG_EXACT		0x00010000
#define	DIG_PRO			0x00020000
#define	DIG_CONSUMER		0x00040000
#define	DIG_PASSTHROUGH		0x00080000
#define	DIG_OUTSEL		0x00100000

	unsigned int valid;
#define	VAL_CBITIN		0x00000001
#define	VAL_UBITIN		0x00000002
#define	VAL_CBITOUT		0x00000004
#define	VAL_UBITOUT		0x00000008
#define	VAL_ISTATUS		0x00000010
#define	VAL_IRATE		0x00000020
#define	VAL_ORATE		0x00000040
#define	VAL_INBITS		0x00000080
#define	VAL_OUTBITS		0x00000100
#define	VAL_REQUEST		0x00000200
#define	VAL_OUTSEL		0x00000400

#define	VAL_OUTMASK (VAL_CBITOUT|VAL_UBITOUT|VAL_ORATE|VAL_OUTBITS|VAL_OUTSEL)

	unsigned int request;
	unsigned int param;
#define	SPD_RQ_PASSTHROUGH	1

	unsigned char cbitin[24];
	unsigned char ubitin[24];
	unsigned char cbitout[24];
	unsigned char ubitout[24];

	unsigned int outsel;
#define	OUTSEL_DIGITAL		1
#define	OUTSEL_ANALOG		2
#define	OUTSEL_BOTH		(OUTSEL_DIGITAL|OUTSEL_ANALOG)

	int in_data;		/* Audio/data if autodetectable by receiver */
#define	IND_UNKNOWN		0
#define	IND_AUDIO		1
#define	IND_DATA		2

	int in_locked;		/* Receiver locked */
#define	LOCK_NOT_INDICATED	0
#define	LOCK_UNLOCKED		1
#define	LOCK_LOCKED		2

	int in_quality;		/* Input signal quality */
#define	IN_QUAL_NOT_INDICATED	0
#define	IN_QUAL_POOR		1
#define	IN_QUAL_GOOD		2

	int in_vbit;
	int out_vbit;		/* V bits */
#define	VBIT_NOT_INDICATED	0
#define	VBIT_OFF		1
#define	VBIT_ON			2

	unsigned int in_errors;	/* Various input error conditions */
#define	INERR_CRC		0x0001
#define	INERR_QCODE_CRC		0x0002
#define	INERR_PARITY		0x0004
#define	INERR_BIPHASE		0x0008

	int srate_in;
	int srate_out;
	int bits_in;
	int bits_out;

	int filler[32];
} oss_digital_control;

/*
 * The "new" mixer API.
 *
 * This improved mixer API makes it possible to access every possible feature
 * of every possible device. However you should read the mixer programming
 * section of the OSS API Developer's Manual. There is no chance that you
 * could use this interface correctly just by examining this header.
 */
#define	OSS_VERSION		0x040003
#define	SOUND_VERSION		OSS_VERSION

typedef struct oss_sysinfo {
	char product[32];	/* E.g. SunOS Audio */
	char version[32];	/* E.g. 4.0a */
	int versionnum;		/* See OSS_GETVERSION */
	char options[128];	/* NOT SUPPORTED */

	int numaudios;		/* # of audio/dsp devices */
	int openedaudio[8];	/* Mask of audio devices are busy */

	int numsynths;		/* NOT SUPPORTED, always 0 */
	int nummidis;		/* NOT SUPPORTED, always 0 */
	int numtimers;		/* NOT SUPPORTED, always 0 */
	int nummixers;		/* # of mixer devices */

	int openedmidi[8];	/* Mask of midi devices are busy */
	int numcards;		/* Number of sound cards in the system */
	int numaudioengines;	/* Number of audio engines in the system */
	char license[16];	/* E.g. "GPL" or "CDDL" */
	char revision_info[256];	/* For internal use */
	int filler[172];	/* For future expansion */
} oss_sysinfo;

typedef struct oss_mixext {
	int dev;		/* Mixer device number */
	int ctrl;		/* Extension number */
	int type;		/* Entry type */
#define	MIXT_DEVROOT		0	/* Device root entry */
#define	MIXT_GROUP		1	/* Controller group */
#define	MIXT_ONOFF		2	/* OFF (0) or ON (1) */
#define	MIXT_ENUM		3	/* Enumerated (0 to maxvalue) */
#define	MIXT_MONOSLIDER		4	/* Mono slider (0 to 255) */
#define	MIXT_STEREOSLIDER	5	/* Stereo slider (dual 0 to 255) */
#define	MIXT_MESSAGE		6	/* (Readable) textual message */
#define	MIXT_MONOVU		7	/* VU meter value (mono) */
#define	MIXT_STEREOVU		8	/* VU meter value (stereo) */
#define	MIXT_MONOPEAK		9	/* VU meter peak value (mono) */
#define	MIXT_STEREOPEAK		10	/* VU meter peak value (stereo) */
#define	MIXT_RADIOGROUP		11	/* Radio button group */
#define	MIXT_MARKER		12	/* Separator between entries */
#define	MIXT_VALUE		13	/* Decimal value entry */
#define	MIXT_HEXVALUE		14	/* Hexadecimal value entry */
#define	MIXT_MONODB		15	/* OBSOLETE */
#define	MIXT_STEREODB		16	/* OBSOLETE */
#define	MIXT_SLIDER		17	/* Slider (mono, 31 bit int range) */
#define	MIXT_3D			18
#define	MIXT_MONOSLIDER16	19	/* Mono slider (0-32767) */
#define	MIXT_STEREOSLIDER16	20	/* Stereo slider (dual 0-32767) */
#define	MIXT_MUTE		21	/* Mute=1, unmute=0 */

	/* Possible value range (minvalue to maxvalue) */
	/* Note that maxvalue may also be smaller than minvalue */
	int maxvalue;
	int minvalue;

	int flags;
#define	MIXF_READABLE	0x00000001	/* Has readable value */
#define	MIXF_WRITEABLE	0x00000002	/* Has writeable value */
#define	MIXF_POLL	0x00000004	/* May change itself */
#define	MIXF_HZ		0x00000008	/* Hertz scale */
#define	MIXF_STRING	0x00000010	/* Use dynamic extensions for value */
#define	MIXF_DYNAMIC	0x00000010	/* Supports dynamic extensions */
#define	MIXF_OKFAIL	0x00000020	/* Interpret value as 1=OK, 0=FAIL */
#define	MIXF_FLAT	0x00000040	/* NOT SUPPORTED */
#define	MIXF_LEGACY	0x00000080	/* NOT SUPPORTED */
#define	MIXF_CENTIBEL	0x00000100	/* Centibel (0.1 dB) step size */
#define	MIXF_DECIBEL	0x00000200	/* Step size of 1 dB */
#define	MIXF_MAINVOL	0x00000400	/* Main volume control */
#define	MIXF_PCMVOL	0x00000800	/* PCM output volume control */
#define	MIXF_RECVOL	0x00001000	/* PCM recording volume control */
#define	MIXF_MONVOL	0x00002000	/* Input->output monitor volume */
#define	MIXF_WIDE	0x00004000	/* NOT SUPPORTED */
#define	MIXF_DESCR	0x00008000	/* NOT SUPPORTED */
#define	MIXF_DISABLE	0x00010000	/* Control has been disabled */

	char id[16];			/* Mnemonic ID (internal use) */
	int parent;			/* Entry# of parent (-1 if root) */

	int dummy;			/* NOT SUPPORTED */

	int timestamp;

	char data[64];			/* Misc data (entry type dependent) */
	unsigned char enum_present[32];	/* Mask of allowed enum values */
	int control_no;			/* NOT SUPPORTED, always -1 */

	unsigned int desc;		/* Scope flags, etc */
#define	MIXEXT_SCOPE_MASK		0x0000003f
#define	MIXEXT_SCOPE_OTHER		0x00000000
#define	MIXEXT_SCOPE_INPUT		0x00000001
#define	MIXEXT_SCOPE_OUTPUT		0x00000002
#define	MIXEXT_SCOPE_MONITOR		0x00000003
#define	MIXEXT_SCOPE_RECSWITCH		0x00000004

	char extname[32];
	int update_counter;
#ifdef	_KERNEL
	int filler[6];
	int enumbit;
#else
	int filler[7];
#endif
} oss_mixext;

typedef struct oss_mixext_root {
	char id[16];
	char name[48];
} oss_mixext_root;

typedef struct oss_mixer_value {
	int dev;
	int ctrl;
	int value;
	int flags;		/* Reserved for future use. Initialize to 0 */
	int timestamp;		/* Must be set to oss_mixext.timestamp */
	int filler[8];		/* Reserved for future use. Initialize to 0 */
} oss_mixer_value;

#define	OSS_LONGNAME_SIZE	64
#define	OSS_LABEL_SIZE		16
#define	OSS_DEVNODE_SIZE	32
typedef	char	oss_longname_t[OSS_LONGNAME_SIZE];
typedef	char	oss_label_t[OSS_LABEL_SIZE];
typedef	char	oss_devnode_t[OSS_DEVNODE_SIZE];


typedef struct oss_audioinfo {
	int dev;		/* Audio device number */
	char name[64];
	int busy;		/* 0, OPEN_READ, OPEN_WRITE, OPEN_READWRITE */
	int pid;		/* Process ID, not used in SunOS */
	int caps;		/* PCM_CAP_INPUT, PCM_CAP_OUTPUT */
	int iformats;		/* Supported input formats */
	int oformats;		/* Supported output formats */
	int magic;		/* Internal use only */
	char cmd[64];		/* Command using the device (if known) */
	int card_number;
	int port_number;
	int mixer_dev;
	int legacy_device;	/* Obsolete field. Replaced by devnode */
	int enabled;		/* 1=enabled, 0=device not ready */
	int flags;		/* internal use only - no practical meaning */
	int min_rate;		/* Minimum sample rate */
	int max_rate;		/* Maximum sample rate */
	int min_channels;	/* Minimum number of channels */
	int max_channels;	/* Maximum number of channels */
	int binding;		/* DSP_BIND_FRONT, etc. 0 means undefined */
	int rate_source;
	char handle[32];
#define	OSS_MAX_SAMPLE_RATES	20	/* Cannot be changed  */
	unsigned int nrates;	/* Array of supported sample rates */
	unsigned int rates[OSS_MAX_SAMPLE_RATES];
	oss_longname_t song_name;	/* Song name (if given) */
	oss_label_t label;	/* Device label (if given) */
	int latency;		/* In usecs, -1=unknown */
	oss_devnode_t devnode;	/* Device special file name (absolute path) */
	int next_play_engine;
	int next_rec_engine;
	int filler[184];
} oss_audioinfo;

typedef struct oss_mixerinfo {
	int dev;
	char id[16];
	char name[32];
	int modify_counter;
	int card_number;
	int port_number;
	char handle[32];
	int magic;		/* Reserved */
	int enabled;		/* Reserved */
	int caps;
#define	MIXER_CAP_VIRTUAL	0x00000001
#define	MIXER_CAP_LAYOUT_B	0x00000002	/* For internal use only */
#define	MIXER_CAP_NARROW	0x00000004	/* Conserve horiz space */
	int flags;		/* Reserved */
	int nrext;
	/*
	 * The priority field can be used to select the default
	 * (motherboard) mixer device. The mixer with the highest
	 * priority is the most preferred one. -2 or less means that
	 * this device cannot be used as the default mixer.
	 */
	int priority;
	oss_devnode_t devnode;  /* Device special file name (absolute path) */
	int legacy_device;
	int filler[245];	/* Reserved */
} oss_mixerinfo;

typedef struct oss_card_info {
	int card;
	char shortname[16];
	char longname[128];
	int flags;
	char hw_info[400];
	int intr_count;
	int ack_count;
	int filler[154];
} oss_card_info;

typedef struct mixer_info {	/* OBSOLETE */
	char id[16];
	char name[32];
	int modify_counter;
	int card_number;
	int port_number;
	char handle[32];
} mixer_info;

#define	MAX_PEAK_CHANNELS	128
typedef unsigned short oss_peaks_t[MAX_PEAK_CHANNELS];

/* For use with SNDCTL_DSP_GET_CHNORDER */
#define	CHID_UNDEF		0
#define	CHID_L			1
#define	CHID_R			2
#define	CHID_C			3
#define	CHID_LFE		4
#define	CHID_LS			5
#define	CHID_RS			6
#define	CHID_LR			7
#define	CHID_RR			8
#define	CHNORDER_UNDEF		0x0000000000000000ULL
#define	CHNORDER_NORMAL		0x0000000087654321ULL


#define	OSSIOCPARM_MASK	0x1fff		/* parameters must be < 8192 bytes */
#define	OSSIOC_VOID	0x00000000	/* no parameters */
#define	OSSIOC_OUT	0x20000000	/* copy out parameters */
#define	OSSIOC_IN	0x40000000	/* copy in parameters */
#define	OSSIOC_INOUT	(OSSIOC_IN|OSSIOC_OUT)
#define	OSSIOC_SZ(t)	((sizeof (t) & OSSIOCPARM_MASK) << 16)
#define	OSSIOC_GETSZ(x)	(((x) >> 16) & OSSIOCPARM_MASK)

#define	__OSSIO(x, y)		((int)(OSSIOC_VOID|(x<<8)|y))
#define	__OSSIOR(x, y, t)	((int)(OSSIOC_OUT|OSSIOC_SZ(t)|(x<<8)|y))
#define	__OSSIOW(x, y, t)	((int)(OSSIOC_IN|OSSIOC_SZ(t)|(x<<8)|y))
#define	__OSSIOWR(x, y, t)	((int)(OSSIOC_INOUT|OSSIOC_SZ(t)|(x<<8)|y))

#define	SNDCTL_SYSINFO		__OSSIOR('X', 1, oss_sysinfo)
#define	OSS_SYSINFO		SNDCTL_SYSINFO  /* Old name */

#define	SNDCTL_MIX_NRMIX	__OSSIOR('X', 2, int)
#define	SNDCTL_MIX_NREXT	__OSSIOWR('X', 3, int)
#define	SNDCTL_MIX_EXTINFO	__OSSIOWR('X', 4, oss_mixext)
#define	SNDCTL_MIX_READ		__OSSIOWR('X', 5, oss_mixer_value)
#define	SNDCTL_MIX_WRITE	__OSSIOWR('X', 6, oss_mixer_value)

#define	SNDCTL_AUDIOINFO	__OSSIOWR('X', 7, oss_audioinfo)
#define	SNDCTL_MIX_ENUMINFO	__OSSIOWR('X', 8, oss_mixer_enuminfo)
#define	SNDCTL_MIDIINFO		__OSSIO('X', 9)
#define	SNDCTL_MIXERINFO	__OSSIOWR('X', 10, oss_mixerinfo)
#define	SNDCTL_CARDINFO		__OSSIOWR('X', 11, oss_card_info)
#define	SNDCTL_ENGINEINFO	__OSSIOWR('X', 12, oss_audioinfo)
#define	SNDCTL_AUDIOINFO_EX	__OSSIOWR('X', 13, oss_audioinfo)
#define	SNDCTL_MIX_DESCRIPTION	__OSSIOWR('X', 14, oss_mixer_enuminfo)

/* ioctl codes 'X', 200-255 are reserved for internal use */

/*
 * Few more "globally" available ioctl calls.
 */
#define	SNDCTL_SETSONG		__OSSIOW('Y', 2, oss_longname_t)
#define	SNDCTL_GETSONG		__OSSIOR('Y', 2, oss_longname_t)
#define	SNDCTL_SETNAME		__OSSIOW('Y', 3, oss_longname_t)
#define	SNDCTL_SETLABEL		__OSSIOW('Y', 4, oss_label_t)
#define	SNDCTL_GETLABEL		__OSSIOR('Y', 4, oss_label_t)

/*
 * IOCTL commands for /dev/dsp
 */
#define	SNDCTL_DSP_HALT		__OSSIO('P', 0)
#define	SNDCTL_DSP_RESET	SNDCTL_DSP_HALT /* Old name */
#define	SNDCTL_DSP_SYNC		__OSSIO('P', 1)
#define	SNDCTL_DSP_SPEED	__OSSIOWR('P', 2, int)

#define	SNDCTL_DSP_STEREO	__OSSIOWR('P', 3, int)	/* OBSOLETE */

#define	SNDCTL_DSP_GETBLKSIZE	__OSSIOWR('P', 4, int)
#define	SNDCTL_DSP_SAMPLESIZE	SNDCTL_DSP_SETFMT
#define	SNDCTL_DSP_CHANNELS	__OSSIOWR('P', 6, int)
#define	SNDCTL_DSP_POST		__OSSIO('P', 8)
#define	SNDCTL_DSP_SUBDIVIDE	__OSSIOWR('P', 9, int)
#define	SNDCTL_DSP_SETFRAGMENT	__OSSIOWR('P', 10, int)

#define	SNDCTL_DSP_GETFMTS	__OSSIOR('P', 11, int)	/* Returns a mask */
#define	SNDCTL_DSP_SETFMT	__OSSIOWR('P', 5, int)	/* Selects ONE fmt */

#define	SNDCTL_DSP_GETOSPACE	__OSSIOR('P', 12, audio_buf_info)
#define	SNDCTL_DSP_GETISPACE	__OSSIOR('P', 13, audio_buf_info)
#define	SNDCTL_DSP_NONBLOCK	__OSSIO('P', 14)	/* Obsolete */
#define	SNDCTL_DSP_GETCAPS	__OSSIOR('P', 15, int)

#define	SNDCTL_DSP_GETTRIGGER	__OSSIOR('P', 16, int)
#define	SNDCTL_DSP_SETTRIGGER	__OSSIOW('P', 16, int)

#define	SNDCTL_DSP_GETIPTR	__OSSIOR('P', 17, count_info)
#define	SNDCTL_DSP_GETOPTR	__OSSIOR('P', 18, count_info)

#define	SNDCTL_DSP_SETSYNCRO	__OSSIO('P', 21)
#define	SNDCTL_DSP_SETDUPLEX	__OSSIO('P', 22)

#define	SNDCTL_DSP_PROFILE	__OSSIOW('P', 23, int)   /* OBSOLETE */
#define	APF_NORMAL	0	/* Normal applications */
#define	APF_NETWORK	1	/* Underruns caused by "external" delay */
#define	APF_CPUINTENS	2	/* Underruns caused by "overheating" the CPU */


#define	SNDCTL_DSP_GETODELAY	__OSSIOR('P', 23, int)

#define	SNDCTL_DSP_GETPLAYVOL	__OSSIOR('P', 24, int)
#define	SNDCTL_DSP_SETPLAYVOL	__OSSIOWR('P', 24, int)
#define	SNDCTL_DSP_GETERROR	__OSSIOR('P', 25, audio_errinfo)

#define	SNDCTL_DSP_READCTL	__OSSIOWR('P', 26, oss_digital_control)
#define	SNDCTL_DSP_WRITECTL	__OSSIOWR('P', 27, oss_digital_control)

#define	SNDCTL_DSP_SYNCGROUP	__OSSIOWR('P', 28, oss_syncgroup)
#define	SNDCTL_DSP_SYNCSTART	__OSSIOW('P', 29, int)

#define	SNDCTL_DSP_COOKEDMODE	__OSSIOW('P', 30, int)

#define	SNDCTL_DSP_SILENCE	__OSSIO('P', 31)
#define	SNDCTL_DSP_SKIP		__OSSIO('P', 32)

#define	SNDCTL_DSP_HALT_INPUT	__OSSIO('P', 33)
#define	SNDCTL_DSP_RESET_INPUT	SNDCTL_DSP_HALT_INPUT   /* Old name */
#define	SNDCTL_DSP_HALT_OUTPUT	__OSSIO('P', 34)
#define	SNDCTL_DSP_RESET_OUTPUT	SNDCTL_DSP_HALT_OUTPUT  /* Old name */

#define	SNDCTL_DSP_LOW_WATER	__OSSIOW('P', 34, int)

#define	SNDCTL_DSP_CURRENT_IPTR	__OSSIOR('P', 35, oss_count_t)
#define	SNDCTL_DSP_CURRENT_OPTR	__OSSIOR('P', 36, oss_count_t)

#define	SNDCTL_DSP_GET_RECSRC_NAMES	__OSSIOR('P', 37, oss_mixer_enuminfo)
#define	SNDCTL_DSP_GET_RECSRC	__OSSIOR('P', 38, int)
#define	SNDCTL_DSP_SET_RECSRC	__OSSIOWR('P', 38, int)

#define	SNDCTL_DSP_GET_PLAYTGT_NAMES	__OSSIOR('P', 39, oss_mixer_enuminfo)
#define	SNDCTL_DSP_GET_PLAYTGT	__OSSIOR('P', 40, int)
#define	SNDCTL_DSP_SET_PLAYTGT	__OSSIOWR('P', 40, int)
#define	SNDCTL_DSP_GETRECVOL	__OSSIOR('P', 41, int)
#define	SNDCTL_DSP_SETRECVOL	__OSSIOWR('P', 41, int)

#define	SNDCTL_DSP_GET_CHNORDER	__OSSIOR('P', 42, unsigned long long)
#define	SNDCTL_DSP_SET_CHNORDER	__OSSIOWR('P', 42, unsigned long long)

#define	SNDCTL_DSP_GETIPEAKS	__OSSIOR('P', 43, oss_peaks_t)
#define	SNDCTL_DSP_GETOPEAKS	__OSSIOR('P', 44, oss_peaks_t)

#define	SNDCTL_DSP_POLICY	__OSSIOW('P', 45, int)    /* See the manual */

#define	SNDCTL_DSP_GETCHANNELMASK	__OSSIOWR('P', 64, int)
#define	SNDCTL_DSP_BIND_CHANNEL	__OSSIOWR('P', 65, int)

/*
 * These definitions are here for the benefit of compiling application
 * code.  Most of these are NOT implemented in the Solaris code,
 * however.  This is the older 3.x OSS API, and only the master input and
 * output levels are actually supported.
 */
#define	SOUND_MIXER_NRDEVICES	28
#define	SOUND_MIXER_VOLUME	0
#define	SOUND_MIXER_BASS	1
#define	SOUND_MIXER_TREBLE	2
#define	SOUND_MIXER_SYNTH	3
#define	SOUND_MIXER_PCM		4
#define	SOUND_MIXER_SPEAKER	5
#define	SOUND_MIXER_LINE	6
#define	SOUND_MIXER_MIC		7
#define	SOUND_MIXER_CD		8
#define	SOUND_MIXER_IMIX	9	/*  Recording monitor  */
#define	SOUND_MIXER_ALTPCM	10
#define	SOUND_MIXER_RECLEV	11	/* Recording level */
#define	SOUND_MIXER_IGAIN	12	/* Input gain */
#define	SOUND_MIXER_OGAIN	13	/* Output gain */
#define	SOUND_MIXER_LINE1	14	/* Input source 1  (aux1) */
#define	SOUND_MIXER_LINE2	15	/* Input source 2  (aux2) */
#define	SOUND_MIXER_LINE3	16	/* Input source 3  (line) */
#define	SOUND_MIXER_DIGITAL1	17	/* Digital I/O 1 */
#define	SOUND_MIXER_DIGITAL2	18	/* Digital I/O 2 */
#define	SOUND_MIXER_DIGITAL3	19	/* Digital I/O 3 */
#define	SOUND_MIXER_PHONE	20	/* Phone */
#define	SOUND_MIXER_MONO	21	/* Mono Output */
#define	SOUND_MIXER_VIDEO	22	/* Video/TV (audio) in */
#define	SOUND_MIXER_RADIO	23	/* Radio in */
#define	SOUND_MIXER_DEPTH	24	/* Surround depth */
#define	SOUND_MIXER_REARVOL	25	/* Rear/Surround speaker vol */
#define	SOUND_MIXER_CENTERVOL	26	/* Center/LFE speaker vol */
#define	SOUND_MIXER_SIDEVOL	27	/* Side-Surround (8speaker) vol */
#define	SOUND_MIXER_SURRVOL	SOUND_MIXER_SIDEVOL
#define	SOUND_ONOFF_MIN		28
#define	SOUND_ONOFF_MAX		30
#define	SOUND_MIXER_NONE	31

#define	SOUND_MIXER_RECSRC	0xff	/* Recording sources */
#define	SOUND_MIXER_DEVMASK	0xfe	/* Supported devices */
#define	SOUND_MIXER_RECMASK	0xfd	/* Recording sources */
#define	SOUND_MIXER_CAPS	0xfc	/* Mixer capabilities (do not use) */
#define	SOUND_MIXER_STEREODEVS	0xfb	/* Mixer channels supporting stereo */
#define	SOUND_MIXER_OUTSRC	0xfa
#define	SOUND_MIXER_OUTMASK	0xf9

#define	SOUND_MIXER_ENHANCE	SOUND_MIXER_NONE
#define	SOUND_MIXER_MUTE	SOUND_MIXER_NONE
#define	SOUND_MIXER_LOUD	SOUND_MIXER_NONE

#define	SOUND_MASK_VOLUME	(1 << SOUND_MIXER_VOLUME)
#define	SOUND_MASK_BASS		(1 << SOUND_MIXER_BASS)
#define	SOUND_MASK_TREBLE	(1 << SOUND_MIXER_TREBLE)
#define	SOUND_MASK_SYNTH	(1 << SOUND_MIXER_SYNTH)
#define	SOUND_MASK_PCM		(1 << SOUND_MIXER_PCM)
#define	SOUND_MASK_SPEAKER	(1 << SOUND_MIXER_SPEAKER)
#define	SOUND_MASK_LINE		(1 << SOUND_MIXER_LINE)
#define	SOUND_MASK_MIC		(1 << SOUND_MIXER_MIC)
#define	SOUND_MASK_CD		(1 << SOUND_MIXER_CD)
#define	SOUND_MASK_IMIX		(1 << SOUND_MIXER_IMIX)
#define	SOUND_MASK_ALTPCM	(1 << SOUND_MIXER_ALTPCM)
#define	SOUND_MASK_RECLEV	(1 << SOUND_MIXER_RECLEV)
#define	SOUND_MASK_IGAIN	(1 << SOUND_MIXER_IGAIN)
#define	SOUND_MASK_OGAIN	(1 << SOUND_MIXER_OGAIN)
#define	SOUND_MASK_LINE1	(1 << SOUND_MIXER_LINE1)
#define	SOUND_MASK_LINE2	(1 << SOUND_MIXER_LINE2)
#define	SOUND_MASK_LINE3	(1 << SOUND_MIXER_LINE3)
#define	SOUND_MASK_DIGITAL1	(1 << SOUND_MIXER_DIGITAL1)
#define	SOUND_MASK_DIGITAL2	(1 << SOUND_MIXER_DIGITAL2)
#define	SOUND_MASK_DIGITAL3	(1 << SOUND_MIXER_DIGITAL3)
#define	SOUND_MASK_MONO		(1 << SOUND_MIXER_MONO)
#define	SOUND_MASK_PHONE	(1 << SOUND_MIXER_PHONE)
#define	SOUND_MASK_RADIO	(1 << SOUND_MIXER_RADIO)
#define	SOUND_MASK_VIDEO	(1 << SOUND_MIXER_VIDEO)
#define	SOUND_MASK_DEPTH	(1 << SOUND_MIXER_DEPTH)
#define	SOUND_MASK_REARVOL	(1 << SOUND_MIXER_REARVOL)
#define	SOUND_MASK_CENTERVOL	(1 << SOUND_MIXER_CENTERVOL)
#define	SOUND_MASK_SIDEVOL	(1 << SOUND_MIXER_SIDEVOL)
#define	SOUND_MASK_SURRVOL	SOUND_MASK_SIDEVOL
#define	SOUND_MASK_MUTE		(1 << SOUND_MIXER_MUTE)
#define	SOUND_MASK_ENHANCE	(1 << SOUND_MIXER_ENHANCE)
#define	SOUND_MASK_LOUD		(1 << SOUND_MIXER_LOUD)

/*
 * Again, DO NOT USE the following two macros.  They are here for SOURCE
 * COMPATIBILITY ONLY.
 */
#define	SOUND_DEVICE_LABELS	{					   \
	"Vol  ", "Bass ", "Treble", "Synth", "Pcm  ", "Speaker ", "Line ", \
	"Mic  ", "CD   ", "Mix  ", "Pcm2 ", "Rec  ", "IGain", "OGain",     \
	"Aux1", "Aux2", "Aux3", "Digital1", "Digital2", "Digital3",	   \
	"Phone", "Mono", "Video", "Radio", "Depth",		           \
	"Rear", "Center", "Side" }

#define	SOUND_DEVICE_NAMES { 						\
	"vol", "bass", "treble", "synth", "pcm", "speaker", "line",	\
	"mic", "cd", "mix", "pcm2", "rec", "igain", "ogain",		\
	"aux1", "aux2", "aux3", "dig1", "dig2", "dig3",			\
	"phone", "mono", "video", "radio", "depth",			\
	"rear", "center", "side" }

#define	MIXER_READ(dev)			__OSSIOR('M', dev, int)
#define	MIXER_WRITE(dev)		__OSSIOWR('M', dev, int)
#define	SOUND_MIXER_INFO		__OSSIOR('M', 101, mixer_info)
#define	OSS_GETVERSION			__OSSIOR('M', 118, int)

/*
 * These macros are useful for some applications.  They are implemented
 * as soft values for the application, and do not affect real hardware.
 */
#define	SOUND_MIXER_READ_VOLUME		MIXER_READ(SOUND_MIXER_VOLUME)
#define	SOUND_MIXER_READ_OGAIN		MIXER_READ(SOUND_MIXER_OGAIN)
#define	SOUND_MIXER_READ_PCM		MIXER_READ(SOUND_MIXER_PCM)
#define	SOUND_MIXER_READ_IGAIN		MIXER_READ(SOUND_MIXER_IGAIN)
#define	SOUND_MIXER_READ_RECLEV		MIXER_READ(SOUND_MIXER_RECLEV)
#define	SOUND_MIXER_READ_RECSRC		MIXER_READ(SOUND_MIXER_RECSRC)
#define	SOUND_MIXER_READ_DEVMASK	MIXER_READ(SOUND_MIXER_DEVMASK)
#define	SOUND_MIXER_READ_RECMASK	MIXER_READ(SOUND_MIXER_RECMASK)
#define	SOUND_MIXER_READ_CAPS		MIXER_READ(SOUND_MIXER_CAPS)
#define	SOUND_MIXER_READ_STEREODEVS	MIXER_READ(SOUND_MIXER_STEREODEVS)
#define	SOUND_MIXER_READ_RECGAIN	__OSSIOR('M', 119, int)
#define	SOUND_MIXER_READ_MONGAIN	__OSSIOR('M', 120, int)

#define	SOUND_MIXER_WRITE_VOLUME	MIXER_WRITE(SOUND_MIXER_VOLUME)
#define	SOUND_MIXER_WRITE_OGAIN		MIXER_WRITE(SOUND_MIXER_OGAIN)
#define	SOUND_MIXER_WRITE_PCM		MIXER_WRITE(SOUND_MIXER_PCM)
#define	SOUND_MIXER_WRITE_IGAIN		MIXER_WRITE(SOUND_MIXER_IGAIN)
#define	SOUND_MIXER_WRITE_RECLEV	MIXER_WRITE(SOUND_MIXER_RECLEV)
#define	SOUND_MIXER_WRITE_RECSRC	MIXER_WRITE(SOUND_MIXER_RECSRC)
#define	SOUND_MIXER_WRITE_RECGAIN	__OSSIOWR('M', 119, int)
#define	SOUND_MIXER_WRITE_MONGAIN	__OSSIOWR('M', 120, int)

/*
 * These macros are here for source compatibility.  They intentionally don't
 * map to any real hardware.  NOT SUPPORTED!
 */
#define	SOUND_MIXER_READ_BASS		MIXER_READ(SOUND_MIXER_BASS)
#define	SOUND_MIXER_READ_TREBLE		MIXER_READ(SOUND_MIXER_TREBLE)
#define	SOUND_MIXER_READ_SYNTH		MIXER_READ(SOUND_MIXER_SYNTH)
#define	SOUND_MIXER_READ_SPEAKER	MIXER_READ(SOUND_MIXER_SPEAKER)
#define	SOUND_MIXER_READ_LINE		MIXER_READ(SOUND_MIXER_LINE)
#define	SOUND_MIXER_READ_MIC		MIXER_READ(SOUND_MIXER_MIC)
#define	SOUND_MIXER_READ_CD		MIXER_READ(SOUND_MIXER_CD)
#define	SOUND_MIXER_READ_IMIX		MIXER_READ(SOUND_MIXER_IMIX)
#define	SOUND_MIXER_READ_ALTPCM		MIXER_READ(SOUND_MIXER_ALTPCM)
#define	SOUND_MIXER_READ_LINE1		MIXER_READ(SOUND_MIXER_LINE1)
#define	SOUND_MIXER_READ_LINE2		MIXER_READ(SOUND_MIXER_LINE2)
#define	SOUND_MIXER_READ_LINE3		MIXER_READ(SOUND_MIXER_LINE3)

#define	SOUND_MIXER_WRITE_BASS		MIXER_WRITE(SOUND_MIXER_BASS)
#define	SOUND_MIXER_WRITE_TREBLE	MIXER_WRITE(SOUND_MIXER_TREBLE)
#define	SOUND_MIXER_WRITE_SYNTH		MIXER_WRITE(SOUND_MIXER_SYNTH)
#define	SOUND_MIXER_WRITE_SPEAKER	MIXER_WRITE(SOUND_MIXER_SPEAKER)
#define	SOUND_MIXER_WRITE_LINE		MIXER_WRITE(SOUND_MIXER_LINE)
#define	SOUND_MIXER_WRITE_MIC		MIXER_WRITE(SOUND_MIXER_MIC)
#define	SOUND_MIXER_WRITE_CD		MIXER_WRITE(SOUND_MIXER_CD)
#define	SOUND_MIXER_WRITE_IMIX		MIXER_WRITE(SOUND_MIXER_IMIX)
#define	SOUND_MIXER_WRITE_ALTPCM	MIXER_WRITE(SOUND_MIXER_ALTPCM)
#define	SOUND_MIXER_WRITE_LINE1		MIXER_WRITE(SOUND_MIXER_LINE1)
#define	SOUND_MIXER_WRITE_LINE2		MIXER_WRITE(SOUND_MIXER_LINE2)
#define	SOUND_MIXER_WRITE_LINE3		MIXER_WRITE(SOUND_MIXER_LINE3)

/*
 * Audio encoding types (Note! U8=8 and S16_LE=16 for compatibility)
 */
#define	AFMT_QUERY	0x00000000	/* Return current fmt */
#define	AFMT_MU_LAW	0x00000001
#define	AFMT_A_LAW	0x00000002
#define	AFMT_IMA_ADPCM	0x00000004
#define	AFMT_U8		0x00000008
#define	AFMT_S16_LE	0x00000010
#define	AFMT_S16_BE	0x00000020
#define	AFMT_S8		0x00000040
#define	AFMT_U16_LE	0x00000080
#define	AFMT_U16_BE	0x00000100
#define	AFMT_MPEG	0x00000200	/* NOT SUPPORTED: MPEG (2) audio */
#define	AFMT_AC3	0x00000400	/* NOT SUPPORTED: AC3 compressed */
#define	AFMT_VORBIS	0x00000800	/* NOT SUPPORTED: Ogg Vorbis */
#define	AFMT_S32_LE	0x00001000
#define	AFMT_S32_BE	0x00002000
#define	AFMT_FLOAT	0x00004000	/* NOT SUPPORTED: IEEE double float */
#define	AFMT_S24_LE	0x00008000	/* LSB aligned in 32 bit word */
#define	AFMT_S24_BE	0x00010000	/* LSB aligned in 32 bit word */
#define	AFMT_SPDIF_RAW	0x00020000	/* NOT SUPPORTED: Raw S/PDIF frames */
#define	AFMT_S24_PACKED	0x00040000	/* 24 bit packed little endian */
/*
 * Some big endian/little endian handling macros (native endian and
 * opposite endian formats).
 */
#if defined(_BIG_ENDIAN)
#define	AFMT_S16_NE	AFMT_S16_BE
#define	AFMT_U16_NE	AFMT_U16_BE
#define	AFMT_S32_NE	AFMT_S32_BE
#define	AFMT_S24_NE	AFMT_S24_BE
#define	AFMT_S16_OE	AFMT_S16_LE
#define	AFMT_S32_OE	AFMT_S32_LE
#define	AFMT_S24_OE	AFMT_S24_LE
#else
#define	AFMT_S16_NE	AFMT_S16_LE
#define	AFMT_U16_NE	AFMT_U16_LE
#define	AFMT_S32_NE	AFMT_S32_LE
#define	AFMT_S24_NE	AFMT_S24_LE
#define	AFMT_S16_OE	AFMT_S16_BE
#define	AFMT_S32_OE	AFMT_S32_BE
#define	AFMT_S24_OE	AFMT_S24_BE
#endif

/*
 * SNDCTL_DSP_GETCAPS bits
 */
#define	PCM_CAP_REVISION	0x000000ff	/* Revision level (0 to 255) */
#define	PCM_CAP_DUPLEX		0x00000100	/* Full duplex rec/play */
#define	PCM_CAP_REALTIME	0x00000200	/* NOT SUPPORTED */
#define	PCM_CAP_BATCH		0x00000400	/* NOT SUPPORTED */
#define	PCM_CAP_COPROC		0x00000800	/* NOT SUPPORTED */
#define	PCM_CAP_TRIGGER		0x00001000	/* Supports SETTRIGGER */
#define	PCM_CAP_MMAP		0x00002000	/* Supports mmap() */
#define	PCM_CAP_MULTI		0x00004000	/* Supports multiple open */
#define	PCM_CAP_BIND		0x00008000	/* Supports channel binding */
#define	PCM_CAP_INPUT		0x00010000	/* Supports recording */
#define	PCM_CAP_OUTPUT		0x00020000	/* Supports playback */
#define	PCM_CAP_VIRTUAL		0x00040000	/* Virtual device */
#define	PCM_CAP_ANALOGOUT	0x00100000	/* NOT SUPPORTED */
#define	PCM_CAP_ANALOGIN	0x00200000	/* NOT SUPPORTED */
#define	PCM_CAP_DIGITALOUT	0x00400000	/* NOT SUPPORTED */
#define	PCM_CAP_DIGITALIN	0x00800000	/* NOT SUPPORTED */
#define	PCM_CAP_ADMASK		0x00f00000	/* NOT SUPPORTED */
#define	PCM_CAP_SHADOW		0x01000000	/* "Shadow" device */
#define	PCM_CAP_CH_MASK		0x06000000	/* See DSP_CH_MASK below */
#define	PCM_CAP_HIDDEN		0x08000000	/* NOT SUPPORTED */
#define	PCM_CAP_FREERATE	0x10000000
#define	PCM_CAP_MODEM		0x20000000	/* NOT SUPPORTED */
#define	PCM_CAP_DEFAULT		0x40000000	/* "Default" device */

/*
 * Preferred channel usage. These bits can be used to give
 * recommendations to the application. Used by few drivers.  For
 * example if ((caps & DSP_CH_MASK) == DSP_CH_MONO) means that the
 * device works best in mono mode. However it doesn't necessarily mean
 * that the device cannot be used in stereo. These bits should only be
 * used by special applications such as multi track hard disk
 * recorders to find out the initial setup. However the user should be
 * able to override this selection.
 *
 * To find out which modes are actually supported the application
 * should try to select them using SNDCTL_DSP_CHANNELS.
 */
#define	DSP_CH_MASK		0x06000000	/* Mask */
#define	DSP_CH_ANY		0x00000000	/* No preferred mode */
#define	DSP_CH_MONO		0x02000000
#define	DSP_CH_STEREO		0x04000000
#define	DSP_CH_MULTI		0x06000000	/* More than two channels */


/*
 * The PCM_CAP_* capability names used to be known as DSP_CAP_*, so
 * it's necessary to define the older names too.
 */
#define	DSP_CAP_ADMASK		PCM_CAP_ADMASK
#define	DSP_CAP_ANALOGIN	PCM_CAP_ANALOGIN
#define	DSP_CAP_ANALOGOUT	PCM_CAP_ANALOGOUT
#define	DSP_CAP_BATCH		PCM_CAP_BATCH
#define	DSP_CAP_BIND		PCM_CAP_BIND
#define	DSP_CAP_COPROC		PCM_CAP_COPROC
#define	DSP_CAP_DEFAULT		PCM_CAP_DEFAULT
#define	DSP_CAP_DIGITALIN	PCM_CAP_DIGITALIN
#define	DSP_CAP_DIGITALOUT	PCM_CAP_DIGITALOUT
#define	DSP_CAP_DUPLEX		PCM_CAP_DUPLEX
#define	DSP_CAP_FREERATE	PCM_CAP_FREERATE
#define	DSP_CAP_HIDDEN		PCM_CAP_HIDDEN
#define	DSP_CAP_INPUT		PCM_CAP_INPUT
#define	DSP_CAP_MMAP		PCM_CAP_MMAP
#define	DSP_CAP_MODEM		PCM_CAP_MODEM
#define	DSP_CAP_MULTI		PCM_CAP_MULTI
#define	DSP_CAP_OUTPUT		PCM_CAP_OUTPUT
#define	DSP_CAP_REALTIME	PCM_CAP_REALTIME
#define	DSP_CAP_REVISION	PCM_CAP_REVISION
#define	DSP_CAP_SHADOW		PCM_CAP_SHADOW
#define	DSP_CAP_TRIGGER		PCM_CAP_TRIGGER
#define	DSP_CAP_VIRTUAL		PCM_CAP_VIRTUAL

/*
 * SNDCTL_DSP_GETTRIGGER and SNDCTL_DSP_SETTRIGGER
 */
#define	PCM_ENABLE_INPUT	0x00000001
#define	PCM_ENABLE_OUTPUT	0x00000002

/*
 * SNDCTL_DSP_BIND_CHANNEL
 */
#define	DSP_BIND_QUERY		0x00000000
#define	DSP_BIND_FRONT		0x00000001
#define	DSP_BIND_SURR		0x00000002
#define	DSP_BIND_CENTER_LFE	0x00000004
#define	DSP_BIND_HANDSET	0x00000008
#define	DSP_BIND_MIC		0x00000010
#define	DSP_BIND_MODEM1		0x00000020
#define	DSP_BIND_MODEM2		0x00000040
#define	DSP_BIND_I2S		0x00000080
#define	DSP_BIND_SPDIF		0x00000100
#define	DSP_BIND_REAR		0x00000200

/*
 * SOUND_MIXER_READ_CAPS
 */
#define	SOUND_CAP_EXCL_INPUT	0x00000001
#define	SOUND_CAP_NOLEGACY	0x00000004
#define	SOUND_CAP_NORECSRC	0x00000008

/*
 * The following ioctl is for internal use only -- it is used to
 * coordinate /dev/sndstat numbering with file names in /dev/sound.
 * Applications must not use it.  (This is duplicated in sys/audioio.h
 * as well.)
 */
#define	SNDCTL_SUN_SEND_NUMBER	__OSSIOW('X', 200, int)

#ifdef __cplusplus
}
#endif

#endif /* _SYS_AUDIO_OSS_H */
