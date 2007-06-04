@expack.exe > NUL 2>&1
@if errorlevel 9009 (
	@echo expack was not found; please add it to your PATH environment variable
	@echo and run this script again.
	@echo.
	@exit
)

@ucc.exe -o usecode %0 > NUL 2>&1
@if errorlevel 9009 (
	@echo ucc was not found; please add it to your PATH environment variable
	@echo and run this script again.
	@echo.
	@exit
)

@for %%f in (faces gumps shapes paperdol sprites fonts mainshp) do @if exist "graphics\%%f.in" (
	@echo Generating %%f...
	@expack.exe -i graphics/%%f.in > NUL: 2>&1
	@del ..\patch\%%f_*.h > NUL: 2>&1
)

@echo Compiling Usecode...
@ucc.exe -o ../patch/usecode usecode.uc
@if errorlevel 1 (
	@echo There were error^(s^) compiling usecode!
) else (
	@echo Usecode has been successfuly compiled!
)
