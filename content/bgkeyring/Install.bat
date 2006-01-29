@echo Copying files to patch folder...
@if EXIST ..\patch goto patchexists
@mkdir ..\patch >> log.txt
:patchexists
@if EXIST ..\patch\map01 goto map01exists
@mkdir ..\patch\map01 >> log.txt
:map01exists
@copy /y data\*.* ..\patch >> log.txt
@copy /y data\map01\*.* ..\patch\map01 >> log.txt && del log.txt
@echo Mod has been successfully installed!
