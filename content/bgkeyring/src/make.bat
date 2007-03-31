@expack.exe >> log.txt 2<&1
@if errorlevel 9009 (
	@echo expack was not found; please add it to your PATH environment variable
	@echo and run this script again.
	@echo.
	@exit
)

@for %%f in (faces gumps shapes paperdol sprites fonts mainshp) do @if exist "graphics\%%f.in" (
	@echo Generating %%f...
	@expack.exe -i graphics/%%f.in >> log.txt
	@del ..\data\%%f_*.h >> log.txt
)

@if exist usecode.uc (
	@ucc.exe -o usecode %0 >> log.txt 2<&1
	@if errorlevel 9009 (
		@echo ucc was not found; please add it to your PATH environment variable
		@echo and run this script again.
		@echo.
		@exit
	)

	@echo Compiling Usecode...
	@ucc.exe -o ../data/usecode usecode.uc
	@if errorlevel 1 (
		@echo There were error^(s^) compiling usecode!
	) else (
		@echo Usecode has been successfuly compiled!
	)
)

@del log.txt
