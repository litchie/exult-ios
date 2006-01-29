@rem ============================================================================================
@rem	This batch installation file was designed by Marzo Junior.
@rem ============================================================================================

@set ipackpath=..\..\..\tools\ipack.exe
@if not exist %ipackpath% goto ipackerror
@if not exist "..\..\static\shapes.vga" goto staticerror

@mkdir Temp >> log.txt

@set blank=graphics\Blank.png
@set filenames=Laurianna Rooted_Laurianna Jaana Mariah

@for /L %%i in (0,1,9) do @for %%f in (%filenames%) do @copy /y %blank% Temp\%%f_0%%i.png >> log.txt
@for /L %%i in (10,1,31) do @for %%f in (%filenames%) do @copy /y %blank% Temp\%%f_%%i.png >> log.txt

@rem ============================================================================================
@rem	Extract scripts. Make sure that, in the scripts, the target dir is temp/ and source dir
@rem	is ../../static/
@rem ============================================================================================
@set faces_extract=ipackscripts\extract_faces.ipk
@set fonts_extract=ipackscripts\extract_fonts.ipk
@set gumps_extract=ipackscripts\extract_gumps.ipk
@set paperdol_extract=ipackscripts\extract_paperdol.ipk
@set shapes_extract=ipackscripts\extract_shapes.ipk
@set sprites_extract=ipackscripts\extract_sprites.ipk

@rem ============================================================================================
@rem	Patch extract scripts. Make sure that, in the scripts, the target dir is temp/ and
@rem	source dir is ../data/
@rem ============================================================================================
@set patch_faces_extract=ipackscripts\patch_faces.ipk
@set patch_fonts_extract=ipackscripts\patch_fonts.ipk
@set patch_gumps_extract=ipackscripts\patch_gumps.ipk
@set patch_paperdol_extract=ipackscripts\patch_paperdol.ipk
@set patch_shapes_extract=ipackscripts\patch_shapes.ipk
@set patch_sprites_extract=ipackscripts\patch_sprites.ipk

@rem ============================================================================================
@rem	New shape scripts. Make sure that, in the scripts, the target dir is ../data/ and
@rem	source dir is temp/
@rem ============================================================================================
@set new_faces=ipackscripts\new_faces.ipk
@set new_fonts=ipackscripts\new_fonts.ipk
@set new_gumps=ipackscripts\new_gumps.ipk
@set new_paperdol=ipackscripts\new_paperdol.ipk
@set new_shapes=ipackscripts\new_shapes.ipk
@set new_sprites=ipackscripts\new_sprites.ipk

@rem ============================================================================================
@rem	Ipack command line parameter (n is a special value of this batch file that prevents
@rem	calling ipack at all).
@rem ============================================================================================
@set varfaces=n
@set varfonts=n
@set vargumps=n
@set varpaperdol=n
@set varshapes=n
@set varsprites=n

:xtractfaces
@if not exist %faces_extract% goto copyfaces
@echo Extracting shapes from static\faces.vga...
@%ipackpath% -x %faces_extract% >> log.txt

:copyfaces
@if not exist %new_faces% goto xtractfonts
@set varfaces=c

@if not exist "..\..\patch\faces.vga" goto xtractfonts
@echo Found faces.vga in \Patch dir; using it as base...
@copy /y ..\..\patch\faces.vga ..\data\faces.vga >> log.txt
@set varfaces=u

@if not exist %patch_faces_extract% goto xtractfonts
@echo Extracting shapes from patch\faces.vga...
@%ipackpath% -x %patch_faces_extract% >> log.txt

:xtractfonts
@if not exist %fonts_extract% goto copyfonts
@echo Extracting shapes from static\fonts.vga...
@%ipackpath% -x %fonts_extract% >> log.txt

:copyfonts
@if not exist %new_fonts% goto xtractgumps
@set varfonts=c

@if not exist "..\..\patch\fonts.vga" goto xtractgumps
@echo Found fonts.vga in \Patch dir; using it as base...
@copy /y ..\..\patch\fonts.vga ..\data\fonts.vga >> log.txt
@set varfonts=u

@if not exist %patch_fonts_extract% goto xtractgumps
@echo Extracting shapes from patch\fonts.vga...
@%ipackpath% -x %patch_fonts_extract% >> log.txt

:xtractgumps
@if not exist %gumps_extract% goto copygumps
@echo Extracting shapes from static\gumps.vga...
@%ipackpath% -x %gumps_extract% >> log.txt

:copygumps
@if not exist %new_gumps% goto xtractpaperdol
@set vargumps=c

@if not exist "..\..\patch\gumps.vga" goto xtractpaperdol
@echo Found gumps.vga in \Patch dir; using it as base...
@copy /y ..\..\patch\gumps.vga ..\data\gumps.vga >> log.txt
@set vargumps=u

@if not exist %patch_gumps_extract% goto xtractpaperdol
@echo Extracting shapes from patch\gumps.vga...
@%ipackpath% -x %patch_gumps_extract% >> log.txt

:xtractpaperdol
@if not exist %paperdol_extract% goto copypaperdol
@echo Extracting shapes from static\paperdol.vga...
@%ipackpath% -x %paperdol_extract% >> log.txt

:copypaperdol
@if not exist %new_paperdol% goto xtractshapes
@set varpaperdol=c

@if not exist "..\..\patch\paperdol.vga" goto xtractshapes
@echo Found paperdol.vga in \Patch dir; using it as base...
@copy /y ..\..\patch\paperdol.vga ..\data\paperdol.vga >> log.txt
@set varpaperdol=u

@if not exist %patch_paperdol_extract% goto xtractshapes
@echo Extracting shapes from patch\paperdol.vga...
@%ipackpath% -x %patch_paperdol_extract% >> log.txt

:xtractshapes
@if not exist %shapes_extract% goto copyshapes
@echo Extracting shapes from static\shapes.vga...
@%ipackpath% -x %shapes_extract% >> log.txt

:copyshapes
@if not exist %new_shapes% goto xtractsprites
@set varshapes=c

@if not exist "..\..\patch\shapes.vga" goto xtractsprites
@echo Found shapes.vga in \Patch dir; using it as base...
@copy /y ..\..\patch\shapes.vga ..\data\shapes.vga >> log.txt
@set varshapes=u

@if not exist %patch_shapes_extract% goto xtractsprites
@echo Extracting shapes from patch\shapes.vga...
@%ipackpath% -x %patch_shapes_extract% >> log.txt

:xtractsprites
@if not exist %sprites_extract% goto copysprites
@echo Extracting shapes from static\sprites.vga...
@%ipackpath% -x %sprites_extract% >> log.txt

:copysprites
@if not exist %new_sprites% goto pngcopy
@set varsprites=c

@if not exist "..\..\patch\sprites.vga" goto pngcopy
@echo Found sprites.vga in \Patch dir; using it as base...
@copy /y ..\..\patch\sprites.vga ..\data\sprites.vga >> log.txt
@set varsprites=u

@if not exist %patch_sprites_extract% goto pngcopy
@echo Extracting shapes from patch\sprites.vga...
@%ipackpath% -x %patch_sprites_extract% >> log.txt


@rem ============================================================================================
@rem
@rem	This copies the pngs from the "graphics" folder into the temp dir.
@rem	It also duplicates the 2-handed swing from Jaana, Laurianna and Mariah
@rem	into 1-handed swings for spellcasting purposes.
@rem
@rem ============================================================================================
:pngcopy
@echo Copying mod png's to Temp dir...
@copy /y graphics\*.png Temp\*.png >> log.txt

@set filenames=Laurianna Jaana Mariah

@for %%f in (%filenames%) do @copy /y Temp\%%f_07.png Temp\%%f_04.png >> log.txt && @copy /y Temp\%%f_08.png Temp\%%f_05.png >> log.txt && @copy /y Temp\%%f_09.png Temp\%%f_06.png >> log.txt && @copy /y Temp\%%f_23.png Temp\%%f_20.png >> log.txt && @copy /y Temp\%%f_24.png Temp\%%f_21.png >> log.txt && @copy /y Temp\%%f_25.png Temp\%%f_22.png >> log.txt


@if %varfaces%==n goto writefonts
@echo Writing faces.vga...
@%ipackpath% -%varfaces% %new_faces% >> log.txt

:writefonts
@if %varfonts%==n goto writegumps
@echo Writing fonts.vga...
@%ipackpath% -%varfonts% %new_fonts% >> log.txt

:writegumps
@if %varfonts%==n goto writepaperdol
@echo Writing gumps.vga...
@%ipackpath% -%vargumps% %new_gumps% >> log.txt

:writepaperdol
@if %varpaperdol%==n goto writeshapes
@echo Writing paperdol.vga...
@%ipackpath% -%varpaperdol% %new_paperdol% >> log.txt

:writeshapes
@if %varshapes%==n goto writesprites
@echo Writing shapes.vga...
@%ipackpath% -%varshapes% %new_shapes% >> log.txt

:writesprites
@if %varsprites%==n goto cleanup
@echo Writing sprites.vga...
@%ipackpath% -%varsprites% %new_sprites% >> log.txt

:cleanup
@echo Cleaning up temporary files...
@del temp\*.* /q >> log.txt
@rmdir temp >> log.txt
@del log.txt

@goto finalize

:staticerror
@echo.
@echo.
@echo.
@echo The STATIC dir of %vargametitle% was not found; you are executing the installation script
@echo from the wrong directory. See the readme.txt file for detailed instructions.
@echo.
@pause
@goto finalize

:ipackerror
@echo.
@echo.
@echo.
@echo IPack was not found. For this script to work, it MUST be in Exult\Tools directory.
@echo See the "readme.txt" file for more information.
@echo.
@pause
@goto finalize

:finalize
