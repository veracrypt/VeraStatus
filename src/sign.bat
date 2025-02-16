PATH=%PATH%;%WSDK81%\bin\x86;C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x86

rem sign using SHA-256
signtool sign /v /sha1 86e1d426731e79117452f090188a828426b29b5f /ac GlobalSign_SHA256_EV_CodeSigning_CA.cer /fd sha256 /tr http://timestamp.digicert.com /td SHA256 Release\VeraStatus.exe x64\Release\VeraStatus.exe ARM64\Release\VeraStatus.exe

pause
