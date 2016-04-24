/*
 Legal Notice: Some portions of the source code contained in this file were
 derived from the source code of TrueCrypt 7.1a, which is 
 Copyright (c) 2003-2012 TrueCrypt Developers Association and which is 
 governed by the TrueCrypt License 3.0, also from the source code of
 Encryption for the Masses 2.02a, which is Copyright (c) 1998-2000 Paul Le Roux
 and which is governed by the 'License Agreement for Encryption for the Masses' 
 Modifications and additions to the original source code (contained in this file) 
 and all other portions of this file are Copyright (c) 2013-2016 IDRIX
 and are governed by the Apache License 2.0 the full text of which is
 contained in the file License.txt included in VeraCrypt binary and source
 code distribution packages. */

#pragma once

#define	_WIN32_WINNT 0x0501
#define WINVER _WIN32_WINNT

#include <windows.h>
#include <stdio.h>
#include <tchar.h>


#define VC_IOCTL(CODE) (CTL_CODE (FILE_DEVICE_UNKNOWN, 0x800 + (CODE), METHOD_BUFFERED, FILE_ANY_ACCESS))

#define VC_IOCTL_GET_BOOT_ENCRYPTION_STATUS				VC_IOCTL (18)
#define VC_IOCTL_GET_BOOT_DRIVE_VOLUME_PROPERTIES		VC_IOCTL (22)

#define VOLUME_ID_SIZE	32

#pragma pack (push)
#pragma pack(1)

typedef enum
{
	SetupNone = 0,
	SetupEncryption,
	SetupDecryption
} BootEncryptionSetupMode;


typedef struct
{
	BOOL DeviceFilterActive;

	unsigned short BootLoaderVersion;

	BOOL DriveMounted;
	BOOL VolumeHeaderPresent;
	BOOL DriveEncrypted;

	LARGE_INTEGER BootDriveLength;

	__int64 ConfiguredEncryptedAreaStart;
	__int64 ConfiguredEncryptedAreaEnd;
	__int64 EncryptedAreaStart;
	__int64 EncryptedAreaEnd;

	unsigned int VolumeHeaderSaltCrc32;

	BOOL SetupInProgress;
	BootEncryptionSetupMode SetupMode;
	BOOL TransformWaitingForIdle;

	unsigned int HibernationPreventionCount;

	BOOL HiddenSystem;
	__int64 HiddenSystemPartitionStart;

	// Number of times the filter driver answered that an unencrypted volume
	// is read-only (or mounted an outer/normal TrueCrypt volume as read only)
	unsigned int HiddenSysLeakProtectionCount;

} BootEncryptionStatus;

typedef struct
{
	int driveNo;
	int uniqueId;
	wchar_t wszVolume[260];
	unsigned __int64 diskLength;
	int ea;
	int mode;
	int pkcs5;
	int pkcs5Iterations;
	BOOL hiddenVolume;
	BOOL readOnly;
	BOOL removable;
	BOOL partitionInInactiveSysEncScope;
	unsigned __int32 volumeHeaderFlags;
	unsigned __int64 totalBytesRead;
	unsigned __int64 totalBytesWritten;
	int hiddenVolProtection;
	int volFormatVersion;
	int volumePim;
	wchar_t wszLabel[33];
	BOOL bDriverSetLabel;
	unsigned char volumeID[VOLUME_ID_SIZE];
} VOLUME_PROPERTIES_STRUCT;

#pragma pack (pop)
