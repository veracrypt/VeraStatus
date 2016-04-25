# VeraStatus
Command line tool to get technical information about VeraCrypt mounted volumes and system encryption 

Usage:
- Querying system encryption information: VeraStatus.exe [/sysenc]
- Querying volume encryption information: VeraStatus.exe DriveLetter: (e.g. VeraStatus.exe O:)
- Displaying usage information: VeraStatus.exe /h

The exit code of the process indicates the result of the operation:
- /sysenc or no argument:
 - 0 => system fully encrypted
 - 1 => system partially encrypted
 - 2 => system not encrypted
 - -1 => VeraCrypt driver not found
 - -2 => Error during driver call
 - -3 => Invalid command line argument
- DriveLetter: argument:
 - 0 => drive letter corresponds to a mounted VeraCrypt volume
 - 3 => no VeraCrypt volume is mounted on the specified drive letter
 - -1 => VeraCrypt driver not found
 - -2 => Error during driver call
 - -3 => Invalid command line argument
