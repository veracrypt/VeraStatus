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
#include <strsafe.h>

// Exit codes values
#define VC_STATUS_OK                    0
#define VC_STATUS_NO_DRIVER             -1
#define VC_STATUS_DRIVER_CALL_FAILED    -2
#define VC_STATUS_INVALID_PARAMETER     -3
#define VC_STATUS_SYSENC_PARTIAL         1
#define VC_STATUS_SYSENC_NONE            2
#define VC_STATUS_NOT_VOLUME             3

// possible state values of system encryption
typedef enum
{
    SYSENC_FULL = 0,
    SYSENC_PARTIAL = 1,
    SYSENC_NONE = 2
} eSysEncState;

LPTSTR GetWin32ErrorStr (DWORD dwError)
{
	LPTSTR lpMsgBuf = NULL;
	static TCHAR g_szErrMsg[1024];	

	FormatMessage (
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			      NULL,
			      dwError,
			      MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),	/* Default language */
			      (PWSTR) &lpMsgBuf,
			      0,
			      NULL
	    );

	if (lpMsgBuf)
	{
		LPTSTR szFormatMsg = (LPTSTR) lpMsgBuf;
		size_t msgLen = _tcslen(szFormatMsg);
		// remove ending \r\n
		if (msgLen >= 1 && szFormatMsg[msgLen - 1] == TEXT('\n'))
			szFormatMsg[msgLen - 1] = 0;
		if (msgLen >= 2 && szFormatMsg[msgLen - 2] == TEXT('\r'))
			szFormatMsg[msgLen - 2] = 0;
		StringCchPrintf (g_szErrMsg, ARRAYSIZE (g_szErrMsg), TEXT("0x%.8X: %s"), dwError, szFormatMsg);
	}
	else
	{
		StringCchPrintf (g_szErrMsg, ARRAYSIZE (g_szErrMsg), TEXT("0x%.8X."), dwError);
	}

	if (lpMsgBuf) LocalFree (lpMsgBuf);

	return g_szErrMsg;
}


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
eSysEncState PrintSystemEncryptionInformation (BootEncryptionStatus& status, DWORD cbSize)
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

		// if the size of the status is smaller than the size of the structure, it means the driver is older than version 1.26.13 where the MasterKeyVulnerable field was added
        if (cbSize >= sizeof(BootEncryptionStatus))
        {
            _tprintf(TEXT("Master Key Vulnerable: %s\n"), status.MasterKeyVulnerable ? TEXT("Yes") : TEXT("No"));
        }
    }

    return state;
}

LPCTSTR GetEncryptionAlgorithmName (int ea)
{
	static TCHAR g_szName[128];
    switch (ea)
    {
	case 1: return TEXT("AES");
    case 2: return TEXT("Serpent");
    case 3: return TEXT("Twofish");
    case 4: return TEXT("Camellia");
    case 5: return TEXT("GOST89");
    case 6: return TEXT("Kuznyechik");
    case 7: return TEXT("AES(Twofish)");
    case 8: return TEXT("AES(Twofish(Serpent))");
    case 9: return TEXT("Serpent(AES)");
    case 10: return TEXT("Serpent(Twofish(AES))");
    case 11: return TEXT("Twofish(Serpent)");
	case 12: return TEXT("Camellia(Kuznyechik)");
	case 13: return TEXT("Kuznyechik(Twofish)");
	case 14: return TEXT("Camellia(Serpent)");
	case 15: return TEXT("Kuznyechik(AES)");
	case 16: return TEXT("Kuznyechik(Serpent(Camellia))");
    default: 
		StringCbPrintf (g_szName, sizeof (g_szName), TEXT("Unknown (id = %d)"), ea);
		return g_szName;
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
    case 5: return TEXT("HMAC-STREEBOG");
    default: return TEXT("Unknown");
    }
}

void PrintBuffer (unsigned char* pbData, size_t cbData)
{
    while (cbData--)
    {
        _tprintf(TEXT("%.2X"), *pbData++);
    }
}

void PrintVolumeInformation (VOLUME_PROPERTIES_STRUCT& prop)
{
    // display only relevant fields
    if (prop.wszVolume[0])
    {
		int driveLetter = TEXT('A') + prop.driveNo;
		if (prop.mountDisabled)
			_tprintf(TEXT("Drive Letter: %c (Virtual Device Only)\n"), driveLetter);
		else
			_tprintf(TEXT("Drive Letter: %c\n"), driveLetter);
		_tprintf(TEXT("Virtual Device: \\Device\\VeraCryptVolume%c\n"), driveLetter);
        _tprintf(TEXT("Volume: %s\n"), prop.wszVolume);
        _tprintf(TEXT("Volume ID: ")); PrintBuffer (prop.volumeID, sizeof (prop.volumeID)); _tprintf(TEXT("\n"));
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

BOOL IsDriveLetter (LPCTSTR szName)
{
	BOOL bRet = FALSE;
	if ((_tcslen (szName) == 2) && (szName[1] == TEXT(':')) && (_totupper(szName[0]) >= TEXT('A')) && (_totupper(szName[0]) <= TEXT('Z')))
		bRet = TRUE;
	return bRet;
}

void PrintUsage ()
{
    _tprintf (TEXT("Usage:\n"));
    _tprintf (TEXT("   Querying system encryption information: VeraStatus.exe [/sysenc]\n"));
    _tprintf (TEXT("   Querying volume encryption information: VeraStatus.exe DriveLetter: (e.g. VeraStatus.exe O:)\n"));
    _tprintf (TEXT("   List all mounted volumes: VeraStatus.exe /list\n"));
    _tprintf (TEXT("   Clear volumes master keys from RAM including system encryption ones: VeraStatus.exe /clearkeys\n"));
    _tprintf (TEXT("   Display this help message: VeraStatus.exe /h\n\n"));
    _tprintf (TEXT("The exit code of the process can be one of the following values:\n"));
    _tprintf (TEXT("   0: The system/volume is encrypted.\n"));
    _tprintf (TEXT("   1: [only when /sysenc specified] The system is partially encrypted.\n"));
    _tprintf (TEXT("   2: [only when /sysenc specified] The system is not encrypted.\n"));
    _tprintf (TEXT("   3: [only when DriveLetter: specified] The drive letter doesn't correspond to a mounted VeraCrypt volume.\n"));
    _tprintf (TEXT("  -1: VeraCrypt Windows driver not found.\n"));
    _tprintf (TEXT("  -2: Error occured when calling VeraCrypt Windows driver.\n"));
    _tprintf (TEXT("  -3: Incorrect command line parameter specified.\n"));
    _tprintf (TEXT("\n"));
}


int _tmain (int argc, TCHAR** argv)
{
    int iRet = 0;
    HANDLE hDriver = INVALID_HANDLE_VALUE;
	LONG DriverVersion = 0;
    BOOL bResult;
	DWORD dwResult;

    _tprintf(TEXT("\n"));
    _tprintf(TEXT("Status of VeraCrypt encryption.By Mounir IDRASSI (mounir@idrix.fr)\n"));
    _tprintf(TEXT("Version 1.5 - Copyright (c) 2016-2025 IDRIX\n"));
    _tprintf(TEXT("\n"));

    // connect to the VeraCrypt driver
    hDriver = CreateFileW (L"\\\\.\\VeraCrypt", 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDriver != INVALID_HANDLE_VALUE)
    {
        bResult = DeviceIoControl(hDriver, VC_IOCTL_GET_DRIVER_VERSION, NULL, 0, &DriverVersion, sizeof(DriverVersion), &dwResult, NULL);
        if (bResult == FALSE)
        {
            _tprintf(TEXT("Failed to get VeraCrypt driver version. Error %s\n"), GetWin32ErrorStr(GetLastError()));
            iRet = VC_STATUS_DRIVER_CALL_FAILED;
            goto end;
        }
        
        _tprintf(TEXT("VeraCrypt driver version: %x.%x\n\n"), (int)(unsigned char)(DriverVersion >> 8), (int)(unsigned char)(DriverVersion & 0x000000FF));
        
        if ((argc == 1) || ((argc == 2) && (_tcsicmp (argv[1], TEXT("/sysenc")) == 0)))
        {
            DWORD cbBytesReturned = 0;
            BootEncryptionStatus status;
        
            // Get system encryption status from VeraCrypt driver
            if (DeviceIoControl(hDriver, VC_IOCTL_GET_BOOT_ENCRYPTION_STATUS, NULL, 0, &status, sizeof(status), &cbBytesReturned, NULL))
            {
                eSysEncState state = PrintSystemEncryptionInformation (status, cbBytesReturned);

                if (state != SYSENC_NONE)
                {
                    VOLUME_PROPERTIES_STRUCT prop;
                    UINT16 bootloaderVersion = 0;

                    cbBytesReturned = 0;
                    if (DeviceIoControl (hDriver, VC_IOCTL_GET_BOOT_DRIVE_VOLUME_PROPERTIES, NULL, 0, &prop, sizeof (prop), &cbBytesReturned, NULL))
                    {
                        _tprintf(TEXT("\n"));
                        PrintVolumeInformation (prop);

                        // get the bootloader version
                        cbBytesReturned = 0;
                        if (DeviceIoControl(hDriver, VC_IOCTL_GET_BOOT_LOADER_VERSION, NULL, 0, &bootloaderVersion, sizeof(bootloaderVersion), &cbBytesReturned, NULL))
                        {
                            _tprintf(TEXT("\nBootloader version: %x.%x\n"), (int)(bootloaderVersion >> 8), (int)(bootloaderVersion & 0x00FF));
						}
						else
						{
                            _tprintf(TEXT("Call to VeraCrypt driver (GET_BOOT_LOADER_VERSION) failed with error %s\n"), GetWin32ErrorStr(GetLastError()));
                        }
                    }
                    else
                    {
                        _tprintf(TEXT("Call to VeraCrypt driver (GET_BOOT_DRIVE_VOLUME_PROPERTIES) failed with error %s\n"), GetWin32ErrorStr(GetLastError ()));
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
                _tprintf(TEXT("Call to VeraCrypt driver (GET_BOOT_ENCRYPTION_STATUS) failed with error %s\n"), GetWin32ErrorStr(GetLastError ()));
                iRet = VC_STATUS_DRIVER_CALL_FAILED;
            }
        }
        else if ((argc == 2) && ((_tcsicmp (argv[1], TEXT("/list")) == 0) || IsDriveLetter (argv[1])))
        {
            DWORD cbBytesReturned = 0;
            VOLUME_PROPERTIES_STRUCT prop;
            MOUNT_LIST_STRUCT mlist;			

	        memset (&mlist, 0, sizeof (mlist));
	        if (DeviceIoControl (hDriver, VC_IOCTL_GET_MOUNTED_VOLUMES, &mlist, sizeof (mlist), &mlist, sizeof (mlist), &cbBytesReturned, NULL))
            {
				if (_tcsicmp (argv[1], TEXT("/list")) == 0)
				{
					int count = 0;
					_tprintf(TEXT("Mounted volumes: "));
					for (int i=0; i < 26; i++)
					{
						if (mlist.ulMountedDrives & (1 << i))
						{
							_tprintf(TEXT("%c:, "), TEXT('A') + i);
							count++;
						}
					}

					if (count)
					{
						_puttchar (VK_BACK);
						_puttchar (VK_BACK);
						_tprintf(TEXT("  \n"));
					}
					else
					{
						_tprintf(TEXT("\rNo volumes are currently mounted on this machine.\n"));
					}
				}
				else
				{
					prop.driveNo = _totupper(argv[1][0]) - TEXT('A');    
					if (mlist.ulMountedDrives & (1 << prop.driveNo))
					{
						if (DeviceIoControl (hDriver, VC_IOCTL_GET_VOLUME_PROPERTIES, &prop, sizeof (prop), &prop, sizeof (prop), &cbBytesReturned, NULL))
						{
							PrintVolumeInformation (prop);
						}
						else
						{
							_tprintf(TEXT("Call to VeraCrypt driver (GET_VOLUME_PROPERTIES) failed with error %s\n"), GetWin32ErrorStr(GetLastError ()));
							iRet = VC_STATUS_DRIVER_CALL_FAILED;
						}
					}
					else
					{
						_tprintf(TEXT("Drive letter %s doesn't correspond to a mounted VeraCrypt volume.\n"), argv[1]);
						iRet = VC_STATUS_NOT_VOLUME;                    
					}
				}
            }
            else
            {
                _tprintf(TEXT("Call to VeraCrypt driver (GET_MOUNTED_VOLUMES) failed with error %s\n"), GetWin32ErrorStr(GetLastError ()));
                iRet = VC_STATUS_DRIVER_CALL_FAILED;
            }
        }
        else if ((argc == 2) && ((_tcsicmp (argv[1], TEXT("/?")) == 0 || _tcsicmp (argv[1], TEXT("/h")) == 0 || _tcsicmp (argv[1], TEXT("/help")) == 0)))
        {
            PrintUsage ();
        }
        else if ((argc == 2) && ((_tcsicmp (argv[1], TEXT("/clearkeys")) == 0)))
        {
            DWORD cbBytesReturned = 0;
        
            // Ask VeraCrypt driver to clear Encrypion keys for all mounted volumes (including Encrypted System) from RAM 
			// In case of system encryption, this will freeze the system. For mounted volume, this will render them unusable
            if (DeviceIoControl (hDriver, VC_IOCTL_EMERGENCY_CLEAR_KEYS, NULL, 0, NULL, 0, &cbBytesReturned, NULL))
            {
				_tprintf (TEXT("Keys cleared successfully!\n"));
			}
            else
            {
                _tprintf(TEXT("Call to VeraCrypt driver (VC_IOCTL_EMERGENCY_SYSENC_CLEAR_KEYS) failed with error %s\n"), GetWin32ErrorStr(GetLastError ()));
                iRet = VC_STATUS_DRIVER_CALL_FAILED;
            }
        }
        else
        {
            _tprintf (TEXT("Error: Invalid parameter(s).\n"));
            PrintUsage ();
            iRet = VC_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        _tprintf(TEXT("Failed to connect to VeraCrypt driver. Please check your installation. Error %s\n"), GetWin32ErrorStr(GetLastError ()));
        iRet = VC_STATUS_NO_DRIVER;
    }
end:
    if (hDriver != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hDriver);
    }
    return iRet;
}
