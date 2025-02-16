# VeraStatus

Command line tool to retrieve technical information about VeraCrypt mounted volumes and system encryption status.

## Overview

VeraStatus is a utility that interfaces with the VeraCrypt driver to provide detailed information about:
- System encryption status and progress
- Mounted VeraCrypt volumes
- Encryption algorithms and configuration
- Volume properties and statistics

## Usage

```
VeraStatus.exe [command]
```

### Commands

- `[no arguments]` or `/sysenc` - Query system encryption information
- `DriveLetter:` - Query specific volume information (e.g., `VeraStatus.exe O:`)
- `/list` - List all mounted VeraCrypt volumes
- `/clearkeys` - Clear encryption keys from RAM (including system encryption)
- `/h` or `/?` or `/help` - Display help information

### Exit Codes

The process returns different exit codes based on the operation:

| Code | Description |
|------|-------------|
| 0 | Success - System/volume is fully encrypted |
| 1 | System is partially encrypted (only with `/sysenc`) |
| 2 | System is not encrypted (only with `/sysenc`) |
| 3 | Drive letter doesn't correspond to a mounted VeraCrypt volume |
| -1 | VeraCrypt driver not found |
| -2 | Error occurred when calling VeraCrypt driver |
| -3 | Invalid command line parameter |

## Features

- Detailed system encryption status including:
  - Encryption progress percentage
  - Bootloader version
  - Hidden system status
  - Setup progress
  - Encrypted area information

- Volume information including:
  - Encryption algorithm
  - PKCS-5 PRF details
  - PIM value usage
  - Read/Write statistics
  - Volume ID and properties

## Building

The project requires:
- Windows SDK
- C++ compiler with Windows API support

To build the project, open the solution file in Visual Studio and compile the project.

## Copyright

Copyright (c) 2016-2025 IDRIX  
By Mounir IDRASSI (mounir@idrix.fr)

This project contains code derived from:
- TrueCrypt 7.1a (Copyright (c) 2003-2012 TrueCrypt Developers Association)
- Encryption for the Masses 2.02a (Copyright (c) 1998-2000 Paul Le Roux)

Licensed under Apache License 2.0
