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

#include "defs.h"

// Exit codes values
#define VC_STATUS_OK                    0
#define VC_STATUS_NO_DRIVER             -1
#define VC_STATUS_DRIVER_CALL_FAILED    -2
#define VC_STATUS_INVALID_PARAMETER     -3
#define VC_STATUS_SYSENC_PARTIAL         1
#define VC_STATUS_SYSENC_NONE            2

// possible state values of system encryption
typedef enum
{
    SYSENC_FULL = 0,
    SYSENC_PARTIAL = 1,
    SYSENC_NONE = 2
} eSysEncState;


// get the state of system encryption from the status returned by the driver
eSysEncState GetSystemEncryptionState (BootEncryptionStatus& status)
{
    if (status.DriveMounted || status.DriveEncrypted)
    {
	    if (!status.SetupInProgress
		    && status.ConfiguredEncryptedAreaEnd != 0
		    && status.ConfiguredEncryptedAreaEnd != -1
		    && status.ConfiguredEncryptedAreaStart == status.EncryptedAreaStart
		    && status.ConfiguredEncryptedAreaEnd == status.EncryptedAreaEnd
            )
        {
            return SYSENC_FULL;
        }
	
        if (	status.EncryptedAreaEnd < 0 
		    || status.EncryptedAreaStart < 0
		    || status.EncryptedAreaEnd <= status.EncryptedAreaStart
		    )
        {
            return SYSENC_NONE;
        }

        return SYSENC_PARTIAL;
    }
    else
        return SYSENC_NONE;
}

// print the status of system encryption as returned by the driver
eSysEncState PrintSystemEncryptionInformation (BootEncryptionStatus& status)
{
    double dVal;
    eSysEncState state = GetSystemEncryptionState (status);

    // display details
    _tprintf(TEXT("System Encryption: %s\n"), (status.DriveMounted || status.DriveEncrypted)? TEXT("Yes") : TEXT("No"));
    _tprintf(TEXT("Encryption State: %s\n"), (state == SYSENC_NONE)? TEXT("None") : (state == SYSENC_PARTIAL)? TEXT("Partial") : TEXT("Full"));
    _tprintf(TEXT("Encrypted Portion: "));
    switch (state)
    {
    case SYSENC_NONE:
        _tprintf(TEXT("0%%\n"));
        break;
    case SYSENC_FULL:
        _tprintf(TEXT("100%%\n"));
        break;
    default:
        dVal = (double) (((unsigned __int64)(status.EncryptedAreaEnd - status.EncryptedAreaStart)) + 1);
        dVal /= (double) (((unsigned __int64)(status.ConfiguredEncryptedAreaEnd - status.ConfiguredEncryptedAreaStart)) + 1);
        dVal *= 100.0;
        _tprintf(TEXT("%.2f%%\n"), dVal);
        break;
    }

    if (status.DriveMounted || status.DriveEncrypted)
    {
        _tprintf(TEXT("Bootloader version: %x.%x\n"), (int) (status.BootLoaderVersion >> 8), (int) (status.BootLoaderVersion & 0x00FF));
        _tprintf(TEXT("Drive mounted: %s\n"), status.DriveMounted? TEXT("Yes") : TEXT("No"));
        _tprintf(TEXT("Drive encrypted: %s\n"), status.DriveEncrypted? TEXT("Yes") : TEXT("No"));
        _tprintf(TEXT("Volume Header present: %s\n"), status.VolumeHeaderPresent? TEXT("Yes") : TEXT("No"));
        _tprintf(TEXT("Hidden System: %s\n"), status.HiddenSystem? TEXT("Yes") : TEXT("No"));
        if (status.HiddenSystem)
        {
            _tprintf(TEXT("Hidden System Partition Start: %I64d\n"), status.HiddenSystemPartitionStart);
            _tprintf(TEXT("Hidden System Leak Protection Count: %d\n"), status.HiddenSysLeakProtectionCount);
            _tprintf(TEXT("\n"));
        }

        _tprintf(TEXT("Setup in progress: %s\n"), status.SetupInProgress? TEXT("Yes") : TEXT("No"));
        if (status.SetupInProgress)
        {
            _tprintf(TEXT("Setup Mode: %s\n"), status.SetupMode == SetupEncryption? TEXT("Encrypting") : 
                status.SetupMode == SetupDecryption? TEXT("Decrypting") : TEXT("None"));
        }
        _tprintf(TEXT("Boot drive size: %I64d Bytes\n"), status.BootDriveLength.QuadPart);
        _tprintf(TEXT("Configured Encrypted Area Start: %I64d\n"), status.ConfiguredEncryptedAreaStart);
        _tprintf(TEXT("Encrypted Area Start: %I64d\n"), status.EncryptedAreaStart);
        _tprintf(TEXT("Configured Encrypted Area End: %I64d\n"), status.ConfiguredEncryptedAreaEnd);
        _tprintf(TEXT("Encrypted Area End: %I64d\n"), status.EncryptedAreaEnd);
    }

    return state;
}

LPCTSTR GetEncryptionAlgorithmName (int ea)
{
    switch (ea)
    {
    case 1: return TEXT("AES");
    case 2: return TEXT("Serpent");
    case 3: return TEXT("Twofish");
    case 4: return TEXT("AES(Twofish)");
    case 5: return TEXT("AES(Twofish(Serpent))");
    case 6: return TEXT("Serpent(AES)");
    case 7: return TEXT("Serpent(Twofish(AES))");
    case 8: return TEXT("Twofish(Serpent)");
    default: return TEXT("Unknown");
    }
}

LPCTSTR GetPrfAlgorithmName (int pkcs5)
{
    switch (pkcs5)
    {
    case 1: return TEXT("HMAC-SHA-512");
    case 2: return TEXT("HMAC-Whirlpool");
    case 3: return TEXT("HMAC-SHA-256");
    case 4: return TEXT("HMAC-RIPEMD-160");
    default: return TEXT("Unknown");
    }
}

void PrintVolumeInformation (VOLUME_PROPERTIES_STRUCT& prop)
{
    // display only relevant fields
    if (prop.wszVolume[0])
    {
        _tprintf(TEXT("Drive Letter: %c\n"), TEXT('A') + prop.driveNo);
        _tprintf(TEXT("Volume: %s\n"), prop.wszVolume);
        if (prop.bDriverSetLabel)
        {
            _tprintf(TEXT("Volume Label: %s\n"), prop.wszLabel);
        }
        _tprintf(TEXT("Hidden Volume: %s\n"), prop.hiddenVolume ? TEXT("Yes") : TEXT("No"));
        _tprintf(TEXT("Hidden Volume Protection Enabled: %s\n"), prop.hiddenVolProtection ? TEXT("Yes") : TEXT("No"));
        _tprintf(TEXT("Read Only: %s\n"), prop.readOnly ? TEXT("Yes") : TEXT("No"));
        _tprintf(TEXT("Removable Media: %s\n"), prop.removable ? TEXT("Yes") : TEXT("No"));
        _tprintf(TEXT("partition In Inactive System Encryption Scope: %s\n"), prop.partitionInInactiveSysEncScope ? TEXT("Yes") : TEXT("No"));
        _tprintf(TEXT("Volume Header Flags: 0x%.8X\n"), prop.volumeHeaderFlags);
    }

    _tprintf(TEXT("Encryption Algorithm: %s\n"), GetEncryptionAlgorithmName (prop.ea));
    _tprintf(TEXT("PKCS-5 PRF: %s\n"), GetPrfAlgorithmName (prop.pkcs5));
    _tprintf(TEXT("Custom PIM used: %s\n"), prop.volumePim > 0 ? TEXT("Yes") : TEXT("No"));
    if (prop.volumePim > 0)
    {
        _tprintf(TEXT("Custom PIM value: %d\n"), prop.volumePim);
    }
    _tprintf(TEXT("Iterations number: %d\n"), prop.pkcs5Iterations);
    _tprintf(TEXT("Data Read Since Mount: %I64d Bytes\n"), prop.totalBytesRead);
    _tprintf(TEXT("Data Written Since Mount: %I64d Bytes\n"), prop.totalBytesWritten);

}

void PrintUsage (TCHAR* progName)
{
    _tprintf (TEXT("Usage: %s /sysenc\n\n"), progName);
}


int _tmain (int argc, TCHAR** argv)
{
    int iRet = 0;
    HANDLE hDriver = INVALID_HANDLE_VALUE;

    _tprintf(TEXT("\n"));
    _tprintf(TEXT("Status of VeraCrypt encryption.By Mounir IDRASSI (mounir@idrix.fr)\n"));
    _tprintf(TEXT("Copyright (c) 2016 IDRIX\n"));
    _tprintf(TEXT("\n\n"));

    // connect to the VeraCrypt driver
    hDriver = CreateFileW (L"\\\\.\\VeraCrypt", 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDriver != INVALID_HANDLE_VALUE)
    {
        // for now, we support only querying system encryption status
        if ((argc == 1) || ((argc == 2) && (_tcsicmp (argv[1], TEXT("/sysenc")) == 0)))
        {
            DWORD cbBytesReturned = 0;
            BootEncryptionStatus status;
        
            // Get system encryption status from VeraCrypt driver
            if (DeviceIoControl (hDriver, VC_IOCTL_GET_BOOT_ENCRYPTION_STATUS, NULL, 0, &status, sizeof (status), &cbBytesReturned, NULL))
            {
                eSysEncState state = PrintSystemEncryptionInformation (status);

                if (state != SYSENC_NONE)
                {
                    VOLUME_PROPERTIES_STRUCT prop;

                    cbBytesReturned = 0;
                    if (DeviceIoControl (hDriver, VC_IOCTL_GET_BOOT_DRIVE_VOLUME_PROPERTIES, NULL, 0, &prop, sizeof (prop), &cbBytesReturned, NULL))
                    {
                        _tprintf(TEXT("\n"));
                        PrintVolumeInformation (prop);
                    }
                    else
                    {
                        _tprintf(TEXT("Call to VeraCrypt driver (GET_BOOT_DRIVE_VOLUME_PROPERTIES) failed with error code 0x%.8X.\n"), GetLastError ());
                    }
                }

                // return the state as this process exit code
                switch (state)
                {
                    case SYSENC_FULL: iRet = VC_STATUS_OK; break;
                    case SYSENC_PARTIAL: iRet = VC_STATUS_SYSENC_PARTIAL; break;
                    default: iRet = VC_STATUS_SYSENC_NONE; break;
                }
            }
            else
            {
                _tprintf(TEXT("Call to VeraCrypt driver (GET_BOOT_ENCRYPTION_STATUS) failed with error code 0x%.8X.\n"), GetLastError ());
                iRet = VC_STATUS_DRIVER_CALL_FAILED;
            }
        }
        else
        {
            _tprintf (TEXT("Error: Invalid parameter(s).\n"));
            PrintUsage (argv[0]);
            iRet = VC_STATUS_INVALID_PARAMETER;
        }

        CloseHandle (hDriver);
    }
    else
    {
        _tprintf(TEXT("Failed to connect to VeraCrypt driver. Please check your installation. Error Code: 0x%.8X\n"), GetLastError ());
        iRet = VC_STATUS_NO_DRIVER;
    }

    return iRet;
}
