@expack.exe >> log.txt 2<&1
@if errorlevel 9009 (
	@echo expack was not found; please add it to your PATH environment variable
	@echo and run this script again.
	@echo.
	@exit
)

@ucc.exe -o usecode %0 >> log.txt 2<&1
@if errorlevel 9009 (
	@echo ucc was not found; please add it to your PATH environment variable
	@echo and run this script again.
	@echo.
	@exit
)

@for %%f in (faces gumps shapes paperdol sprites fonts) do @if exist "graphics\%%f.in" (
	@echo Generating %%f.vga...
	@expack.exe -i graphics/%%f.in >> log.txt
	@del ..\data\%%f_vga.h >> log.txt
)

@echo Compiling Usecode...
@ucc.exe -o ../data/usecode usecode.uc
@if errorlevel 1 (
	@echo There were error^(s^) compiling usecode!
) else (
	@echo Usecode has been successfuly compiled!
)

@del log.txt
