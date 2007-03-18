@rem ============================================================================================
@rem	This batch installation file was designed by Marzo Junior.
@rem ============================================================================================

@set expack=expack.exe
@%expack% >> log.txt
@if errorlevel 9009 goto expackerror

@for %%f in (faces gumps shapes paperdol sprites fonts) do @if exist "graphics\%%f.in" (
	@echo Generating %%f.vga...
	@%expack% -i graphics\%%f.in >> log.txt
	@del ..\data\%%f_vga.h >> log.txt
)

@del log.txt

@goto finalize

:expackerror
@echo.
@echo.
@echo.
@echo ExPack was not found. Make sure that you have it installed and add ExPack's
@echo directory to your PATH environment variable.
@echo.
@pause
@goto finalize

:finalize
