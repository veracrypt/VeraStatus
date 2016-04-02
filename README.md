# VeraStatus
Command line tool to get technical information about VeraCrypt mounted volumes and system encryption 

Currently, only querying system encryption information is implemented.
Usage: VeraStatus.exe [/sysenc]

The exit code of the process indicates the result of the operation:
- /sysenc or no argument:
 - 0 => system fully encrypted
 - 1 => system partially encrypted
 - 2 => system not encrypted
 - -1 => VeraCrypt driver not found
 - -2 => Error during driver call
 - -3 => Invalid command line argument
