/* 
 * Copyright (c) [2010-2019] zhenfei.mzf@gmail.com rights reserved.
 * 
 * AntiSpy is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *
 *     http://license.coscl.org.cn/MulanPSL
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
 * FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v1 for more details.
*/
/*
 * volume.h - Exports for NTFS volume handling. Originated from the Linux-NTFS project.
 */

#ifndef _NTFS_VOLUME_H
#define _NTFS_VOLUME_H


#include <stdio.h>
#include <windows.h>
#include <string.h>


/*
 * Under Cygwin, DJGPP and FreeBSD we do not have MS_RDONLY and MS_NOATIME,
 * so we define them ourselves.
 */
#ifndef MS_RDONLY
#define MS_RDONLY 1
#endif
/*
 * Solaris defines MS_RDONLY but not MS_NOATIME thus we need to carefully
 * define MS_NOATIME.
 */
#ifndef MS_NOATIME
#if (MS_RDONLY != 1)
#	define MS_NOATIME 1
#else
#	define MS_NOATIME 2
#endif
#endif

#define MS_EXCLUSIVE 0x08000000

/* Forward declaration */
typedef struct _ntfs_volume ntfs_volume;

#include "types.h"
#include "support.h"
#include "device.h"
#include "inode.h"
#include "attrib.h"

/**
 * enum ntfs_mount_flags -
 *
 * Flags returned by the ntfs_check_if_mounted() function.
 */
typedef enum {
	NTFS_MF_MOUNTED		= 1,	/* Device is mounted. */
	NTFS_MF_ISROOT		= 2,	/* Device is mounted as system root. */
	NTFS_MF_READONLY	= 4,	/* Device is mounted read-only. */
} ntfs_mount_flags;

extern int ntfs_check_if_mounted(const char *file, unsigned long *mnt_flags);

/**
 * enum ntfs_volume_state_bits -
 *
 * Defined bits for the state field in the ntfs_volume structure.
 */
typedef enum {
	NV_ReadOnly,		/* 1: Volume is read-only. */
	NV_CaseSensitive,	/* 1: Volume is mounted case-sensitive. */
	NV_LogFileEmpty,	/* 1: $logFile journal is empty. */
	NV_NoATime,			/* 1: Do not update access time. */
} ntfs_volume_state_bits;

#define  test_nvol_flag(nv, flag)	 test_bit(NV_##flag, (nv)->state)
#define   set_nvol_flag(nv, flag)	  set_bit(NV_##flag, (nv)->state)
#define clear_nvol_flag(nv, flag)	clear_bit(NV_##flag, (nv)->state)

#define NVolReadOnly(nv)		 test_nvol_flag(nv, ReadOnly)
#define NVolSetReadOnly(nv)		  set_nvol_flag(nv, ReadOnly)
#define NVolClearReadOnly(nv)		clear_nvol_flag(nv, ReadOnly)

#define NVolCaseSensitive(nv)		 test_nvol_flag(nv, CaseSensitive)
#define NVolSetCaseSensitive(nv)	  set_nvol_flag(nv, CaseSensitive)
#define NVolClearCaseSensitive(nv)	clear_nvol_flag(nv, CaseSensitive)

#define NVolLogFileEmpty(nv)		 test_nvol_flag(nv, LogFileEmpty)
#define NVolSetLogFileEmpty(nv)		  set_nvol_flag(nv, LogFileEmpty)
#define NVolClearLogFileEmpty(nv)	clear_nvol_flag(nv, LogFileEmpty)

#define NVolNoATime(nv)			 test_nvol_flag(nv, NoATime)
#define NVolSetNoATime(nv)		  set_nvol_flag(nv, NoATime)
#define NVolClearNoATime(nv)		clear_nvol_flag(nv, NoATime)

/*
 * NTFS version 1.1 and 1.2 are used by Windows NT4.
 * NTFS version 2.x is used by Windows 2000 Beta
 * NTFS version 3.0 is used by Windows 2000.
 * NTFS version 3.1 is used by Windows XP, 2003 and Vista.
 */

#define NTFS_V1_1(major, minor) ((major) == 1 && (minor) == 1)
#define NTFS_V1_2(major, minor) ((major) == 1 && (minor) == 2)
#define NTFS_V2_X(major, minor) ((major) == 2)
#define NTFS_V3_0(major, minor) ((major) == 3 && (minor) == 0)
#define NTFS_V3_1(major, minor) ((major) == 3 && (minor) == 1)

#define NTFS_BUF_SIZE 8192

/**
 * struct _ntfs_volume - structure describing an open volume in memory.
 */
struct _ntfs_volume {
	union {
		struct ntfs_device *dev;	/* NTFS device associated with the volume. */
		void *sb;	/* For kernel porting compatibility. */
	};
	char *vol_name;		/* Name of the volume. */
	unsigned long state;	/* NTFS specific flags describing this volume.See ntfs_volume_state_bits above. */
	ntfs_inode *vol_ni;	/* ntfs_inode structure for FILE_Volume. */
	u8 major_ver;		/* Ntfs major version of volume. */
	u8 minor_ver;		/* Ntfs minor version of volume. */
	u16 flags;			/* Bit array of VOLUME_* flags. */

	u16 sector_size;	/* Byte size of a sector. */
	u8 sector_size_bits;	/* Log(2) of the byte size of a sector. */
	u32 cluster_size;	/* Byte size of a cluster. */
	u32 mft_record_size;	/* Byte size of a mft record. */
	u32 indx_record_size;	/* Byte size of a INDX record. */
	u8 cluster_size_bits;	/* Log(2) of the byte size of a cluster. */
	u8 mft_record_size_bits;/* Log(2) of the byte size of a mft record. */
	u8 indx_record_size_bits;/* Log(2) of the byte size of a INDX record. */

	/* Variables used by the cluster and mft allocators. */
	u8 mft_zone_multiplier;	/* Initial mft zone multiplier. */
	s64 mft_data_pos;	/* Mft record number at which to allocate the next mft record. */
	LCN mft_zone_start;	/* First cluster of the mft zone. */
	LCN mft_zone_end;	/* First cluster beyond the mft zone. */
	LCN mft_zone_pos;	/* Current position in the mft zone. */
	LCN data1_zone_pos;	/* Current position in the first data zone. */
	LCN data2_zone_pos;	/* Current position in the second data zone. */

	s64 nr_clusters;	/* Volume size in clusters, hence also the number of bits in lcn_bitmap. */
	ntfs_inode *lcnbmp_ni;	/* ntfs_inode structure for FILE_Bitmap. */
	ntfs_attr *lcnbmp_na;	/* ntfs_attr structure for the data attribute
				   of FILE_Bitmap. Each bit represents a
				   cluster on the volume, bit 0 representing
				   lcn 0 and so on. A set bit means that the
				   cluster and vice versa. */

	LCN mft_lcn;		/* Logical cluster number of the data attribute
				   for FILE_MFT. */
	ntfs_inode *mft_ni;	/* ntfs_inode structure for FILE_MFT. */
	ntfs_attr *mft_na;	/* ntfs_attr structure for the data attribute
				   of FILE_MFT. */
	ntfs_attr *mftbmp_na;	/* ntfs_attr structure for the bitmap attribute
				   of FILE_MFT. Each bit represents an mft
				   record in the $DATA attribute, bit 0
				   representing mft record 0 and so on. A set
				   bit means that the mft record is in use and
				   vice versa. */

	int mftmirr_size;	/* Size of the FILE_MFTMirr in mft records. */
	LCN mftmirr_lcn;	/* Logical cluster number of the data attribute
				   for FILE_MFTMirr. */
	ntfs_inode *mftmirr_ni;	/* ntfs_inode structure for FILE_MFTMirr. */
	ntfs_attr *mftmirr_na;	/* ntfs_attr structure for the data attribute
				   of FILE_MFTMirr. */

	ntfschar *upcase;	/* Upper case equivalents of all 65536 2-byte
				   Unicode characters. Obtained from
				   FILE_UpCase. */
	u32 upcase_len;		/* Length in Unicode characters of the upcase
				   table. */

	ATTR_DEF *attrdef;	/* Attribute definitions. Obtained from
				   FILE_AttrDef. */
	s32 attrdef_len;	/* Size of the attribute definition table in
				   bytes. */

	/* Temp: for directory handling */
	void *private_data;	/* ntfs_dir for . */
	void *private_bmp1;	/* ntfs_bmp for $MFT/$BITMAP */
	void *private_bmp2;	/* ntfs_bmp for $Bitmap */
};

extern ntfs_volume *ntfs_volume_alloc(void);

extern ntfs_volume *ntfs_volume_startup(struct ntfs_device *dev,
		unsigned long flags);

extern ntfs_volume *ntfs_device_mount(struct ntfs_device *dev,
		unsigned long flags);
extern int ntfs_device_umount(ntfs_volume *vol, const BOOL force);

extern ntfs_volume *ntfs_mount(const char *name, unsigned long flags);
extern int ntfs_umount(ntfs_volume *vol, const BOOL force);

extern int ntfs_version_is_supported(ntfs_volume *vol);
extern int ntfs_logfile_reset(ntfs_volume *vol);

extern int ntfs_volume_write_flags(ntfs_volume *vol, const u16 flags);

#endif /* defined _NTFS_VOLUME_H */

