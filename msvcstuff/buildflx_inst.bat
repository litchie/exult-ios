cd "%1" 
\UC\EXULT\tools\expack.exe -i flx.in 
@if not exist "C:\UC\ULTIMA7\%4" md "C:\UC\ULTIMA7\%4" 
cd "%2" 
copy "%3" "C:\UC\ULTIMA7\%4" 
