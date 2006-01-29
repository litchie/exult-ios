@cd src
@set buildvga=%1
@if "%buildvga%"=="-skipvga" goto make_usecode
@call makevgafiles.bat
:make_usecode
@call makeusecode.bat
@cd..
@call install.bat
