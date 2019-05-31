

rem pvk2pfx -spc mycert.spc -pvk mykey.pvk -pfx IZexCert.pfx -po izex1588




rem Windows 7


signtool.exe sign /v /ac MSCV-VSClass3.cer /t http://timestamp.verisign.com/scripts/timestamp.dll  ..\Win7\ko\i386\NHCAFlt.sys
signtool verify /kp ..\Win7\ko\i386\NHCAFlt.sys

signtool.exe sign /v /ac MSCV-VSClass3.cer /t http://timestamp.verisign.com/scripts/timestamp.dll  ..\Win7\en\i386\NHCAFlt.sys
signtool verify /kp ..\Win7\en\i386\NHCAFlt.sys


rem Windows 7 AMD64



signtool.exe sign /v /ac MSCV-VSClass3.cer /t http://timestamp.verisign.com/scripts/timestamp.dll  ..\Win7\ko\amd64\NHCAFlt.sys
signtool verify /kp ..\Win7\ko\amd64\NHCAFlt.sys

signtool.exe sign /v /ac MSCV-VSClass3.cer /t http://timestamp.verisign.com/scripts/timestamp.dll  ..\Win7\en\amd64\NHCAFlt.sys
signtool verify /kp ..\Win7\en\amd64\NHCAFlt.sys




pause