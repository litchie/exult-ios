# Microsoft Developer Studio Project File - Name="Exult" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Exult - Win32 Debug Install
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Exult.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Exult.mak" CFG="Exult - Win32 Debug Install"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Exult - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Exult - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "Exult - Win32 Release Install" (based on "Win32 (x86) Console Application")
!MESSAGE "Exult - Win32 Debug Install" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Exult - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GR /GX /O2 /I "..\\" /I "..\Gumps" /I "..\conf" /I "..\files" /I "..\filc" /I "..\imagewin" /I "..\objs" /I "..\pathfinder" /I "..\shapes" /I "..\usecode" /I "..\MSVCStuff" /I "..\Audio" /I "..\Audio\MIDI_drivers" /I "..\data" /I "..\server" /FI"msvc_kludges.h" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "EXULT" /FR /Yu"msvc_kludges.h" /FD /TP /c
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Release\Exult.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 SDLMain.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib winmm.lib zlib.lib /nologo /version:0.97 /subsystem:windows /incremental:yes /machine:I386 /nodefaultlib:"libc.lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MTd /W3 /Gm /GR /GX /ZI /Od /I "..\\" /I "..\Gumps" /I "..\conf" /I "..\files" /I "..\filc" /I "..\imagewin" /I "..\objs" /I "..\pathfinder" /I "..\shapes" /I "..\usecode" /I "..\MSVCStuff" /I "..\Audio" /I "..\Audio\MIDI_drivers" /I "..\data" /I "..\server" /FI"msvc_kludges.h" /D "_DEBUG" /D "DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "EXULT" /FR /Yu"msvc_kludges.h" /FD /GZ /TP /c
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"debug\Exult.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib winmm.lib zlib.lib /nologo /version:0.97 /subsystem:console /debug /machine:I386 /nodefaultlib:"libc.lib" /pdbtype:sept
# SUBTRACT LINK32 /incremental:no /map

!ELSEIF  "$(CFG)" == "Exult - Win32 Release Install"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Exult___Win32_Release_Install"
# PROP BASE Intermediate_Dir "Exult___Win32_Release_Install"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MD /W3 /GX /O2 /I "..\\" /I "..\Gumps" /I "..\conf" /I "..\files" /I "..\filc" /I "..\imagewin" /I "..\objs" /I "..\pathfinder" /I "..\shapes" /I "..\usecode" /I "..\MSVCStuff" /I "..\SDL-1.2.0\include" /I "..\Audio" /I "..\Audio\MIDI_drivers" /FI"msvc_kludges.h" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "EXULT" /Yu"msvc_kludges.h" /FD /TP /c
# ADD CPP /nologo /G6 /MD /W3 /GR /GX /O2 /I "..\\" /I "..\Gumps" /I "..\conf" /I "..\files" /I "..\filc" /I "..\imagewin" /I "..\objs" /I "..\pathfinder" /I "..\shapes" /I "..\usecode" /I "..\MSVCStuff" /I "..\Audio" /I "..\Audio\MIDI_drivers" /I "..\data" /I "..\server" /FI"msvc_kludges.h" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "EXULT" /FR /Yu"msvc_kludges.h" /FD /TP /c
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Release\Exult.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib SDLMain.lib winmm.lib /nologo /subsystem:windows /machine:I386 /libpath:"..\SDL-1.2.0\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 SDLMain.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib winmm.lib zlib.lib /nologo /version:0.97 /subsystem:windows /incremental:yes /machine:I386 /nodefaultlib:"libc.lib"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - copying $(TargetPath) to $(EXULT_INSTALL_PATH)\$(TargetName).exe
TargetPath=\UC\exult\Exult.exe
TargetName=Exult
InputPath=\UC\exult\Exult.exe
SOURCE="$(InputPath)"

"$(EXULT_INSTALL_PATH)\$(TargetName).exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(TargetPath) $(EXULT_INSTALL_PATH)

# End Custom Build

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug Install"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Exult___Win32_Debug_Install"
# PROP BASE Intermediate_Dir "Exult___Win32_Debug_Install"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G6 /MTd /W3 /Gm /GX /ZI /Od /I "..\\" /I "..\Gumps" /I "..\conf" /I "..\files" /I "..\filc" /I "..\imagewin" /I "..\objs" /I "..\pathfinder" /I "..\shapes" /I "..\usecode" /I "..\MSVCStuff" /I "..\SDL-1.2.0\include" /I "..\Audio" /I "..\Audio\MIDI_drivers" /FI"msvc_kludges.h" /D "_DEBUG" /D "DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "EXULT" /FR /Yu"msvc_kludges.h" /FD /GZ /TP /c
# ADD CPP /nologo /G6 /MTd /W3 /Gm /GR /GX /ZI /Od /I "..\\" /I "..\Gumps" /I "..\conf" /I "..\files" /I "..\filc" /I "..\imagewin" /I "..\objs" /I "..\pathfinder" /I "..\shapes" /I "..\usecode" /I "..\MSVCStuff" /I "..\Audio" /I "..\Audio\MIDI_drivers" /I "..\data" /I "..\server" /FI"msvc_kludges.h" /D "_DEBUG" /D "DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "EXULT" /FR /Yu"msvc_kludges.h" /FD /GZ /TP /c
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"debug\Exult.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib winmm.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"..\SDL-1.2.0\lib"
# SUBTRACT BASE LINK32 /incremental:no /map
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib winmm.lib zlib.lib /nologo /version:0.97 /subsystem:console /debug /machine:I386 /nodefaultlib:"libc.lib" /pdbtype:sept
# SUBTRACT LINK32 /incremental:no /map
# Begin Custom Build - copying $(TargetPath) to $(EXULT_INSTALL_PATH)\$(TargetName).exe
TargetPath=\UC\exult\Exult.exe
TargetName=Exult
InputPath=\UC\exult\Exult.exe
SOURCE="$(InputPath)"

"$(EXULT_INSTALL_PATH)\$(TargetName).exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy $(TargetPath) $(EXULT_INSTALL_PATH)

# End Custom Build

!ENDIF 

# Begin Target

# Name "Exult - Win32 Release"
# Name "Exult - Win32 Debug"
# Name "Exult - Win32 Release Install"
# Name "Exult - Win32 Debug Install"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Audio"

# PROP Default_Filter ""
# Begin Group "Audio Unused"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\audio\midi_drivers\be_midi.cc

!IF  "$(CFG)" == "Exult - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Release Install"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug Install"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\audio\midi_drivers\forked_player.cc

!IF  "$(CFG)" == "Exult - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Release Install"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug Install"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\audio\midi_drivers\KMIDI.cc

!IF  "$(CFG)" == "Exult - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Release Install"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug Install"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\audio\midi_drivers\mac_midi.cc

!IF  "$(CFG)" == "Exult - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Release Install"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug Install"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\audio\midi_drivers\Makefile.am

!IF  "$(CFG)" == "Exult - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Release Install"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug Install"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\audio\midi_drivers\Timidity_binary.cc

!IF  "$(CFG)" == "Exult - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Release Install"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug Install"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\audio\midi_drivers\win_MCI.cc

!IF  "$(CFG)" == "Exult - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Release Install"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug Install"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=..\audio\Audio.cc

!IF  "$(CFG)" == "Exult - Win32 Release"

# ADD CPP /Ze /O2

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug"

!ELSEIF  "$(CFG)" == "Exult - Win32 Release Install"

# ADD BASE CPP /Ze /O2
# ADD CPP /Ze /O2

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug Install"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\audio\conv.cc
# End Source File
# Begin Source File

SOURCE=..\audio\Makefile.am
# End Source File
# Begin Source File

SOURCE=..\audio\Midi.cc
# End Source File
# Begin Source File

SOURCE=..\audio\Mixer.cc
# End Source File
# Begin Source File

SOURCE=..\audio\pcb.cc
# End Source File
# Begin Source File

SOURCE=..\audio\SDL_mapping.cc
# End Source File
# Begin Source File

SOURCE=..\audio\soundtest.cc
# End Source File
# Begin Source File

SOURCE=..\audio\midi_drivers\win_midiout.cc
# End Source File
# Begin Source File

SOURCE=..\audio\xmidi.cc
# End Source File
# End Group
# Begin Group "conf"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\conf\Configuration.cc
# End Source File
# Begin Source File

SOURCE=..\conf\Makefile.am
# End Source File
# Begin Source File

SOURCE=..\conf\README
# End Source File
# Begin Source File

SOURCE=..\conf\XMLEntity.cc
# End Source File
# End Group
# Begin Group "Exult"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\actions.cc
# End Source File
# Begin Source File

SOURCE=..\actorio.cc
# End Source File
# Begin Source File

SOURCE=..\actors.cc
# End Source File
# Begin Source File

SOURCE=..\alloc.cc
# End Source File
# Begin Source File

SOURCE=..\args.cc
# End Source File
# Begin Source File

SOURCE=..\bggame.cc
# End Source File
# Begin Source File

SOURCE=..\browser.cc
# End Source File
# Begin Source File

SOURCE=..\cheat.cc
# End Source File
# Begin Source File

SOURCE=..\cheat_screen.cc
# End Source File
# Begin Source File

SOURCE=..\combat.cc
# End Source File
# Begin Source File

SOURCE=..\delobjs.cc
# End Source File
# Begin Source File

SOURCE=..\dir.cc
# End Source File
# Begin Source File

SOURCE=..\drag.cc
# End Source File
# Begin Source File

SOURCE=..\effects.cc
# End Source File
# Begin Source File

SOURCE=..\exult.cc
# End Source File
# Begin Source File

SOURCE=..\exultmenu.cc
# End Source File
# Begin Source File

SOURCE=..\game.cc
# End Source File
# Begin Source File

SOURCE=..\gameclk.cc
# End Source File
# Begin Source File

SOURCE=..\gamedat.cc
# End Source File
# Begin Source File

SOURCE=..\gamemap.cc
# End Source File
# Begin Source File

SOURCE=..\gamerend.cc
# End Source File
# Begin Source File

SOURCE=..\gamewin.cc
# End Source File
# Begin Source File

SOURCE=..\keyactions.cc
# End Source File
# Begin Source File

SOURCE=..\keys.cc
# End Source File
# Begin Source File

SOURCE=..\Makefile.am
# End Source File
# Begin Source File

SOURCE=..\menulist.cc
# End Source File
# Begin Source File

SOURCE=..\monsters.cc
# End Source File
# Begin Source File

SOURCE=..\mouse.cc
# End Source File
# Begin Source File

SOURCE=..\npcnear.cc
# End Source File
# Begin Source File

SOURCE=..\npctime.cc
# End Source File
# Begin Source File

SOURCE=..\palette.cc
# End Source File
# Begin Source File

SOURCE=..\paths.cc
# End Source File
# Begin Source File

SOURCE=..\readnpcs.cc
# End Source File
# Begin Source File

SOURCE=..\schedule.cc
# End Source File
# Begin Source File

SOURCE=..\segfile.cc
# End Source File
# Begin Source File

SOURCE=..\shapeid.cc
# End Source File
# Begin Source File

SOURCE=..\sigame.cc
# End Source File
# Begin Source File

SOURCE=..\tqueue.cc
# End Source File
# Begin Source File

SOURCE=..\txtscroll.cc
# End Source File
# Begin Source File

SOURCE=..\version.cc
# End Source File
# End Group
# Begin Group "Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\files\crc.cc
# End Source File
# Begin Source File

SOURCE=..\files\Flat.cc
# End Source File
# Begin Source File

SOURCE=..\files\Flex.cc
# End Source File
# Begin Source File

SOURCE=..\files\IFF.cc
# End Source File
# Begin Source File

SOURCE=..\files\listfiles.cc
# End Source File
# Begin Source File

SOURCE=..\files\Makefile.am
# End Source File
# Begin Source File

SOURCE=..\files\Table.cc
# End Source File
# Begin Source File

SOURCE=..\files\U7file.cc
# End Source File
# Begin Source File

SOURCE=..\files\utils.cc
# End Source File
# Begin Source File

SOURCE=..\files\zip\zip_u7file.cc
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "flic"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\flic\Makefile.am
# End Source File
# Begin Source File

SOURCE=..\flic\playfli.cc
# End Source File
# End Group
# Begin Group "gumps"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\gumps\Actor_gump.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\AudioOptions_gump.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Book_gump.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\CombatStats_gump.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Enabled_button.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Face_button.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Face_stats.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\File_gump.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Gamemenu_gump.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\GameplayOptions_gump.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Gump.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Gump_button.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Gump_manager.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Gump_ToggleButton.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\gump_utils.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Gump_widget.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Jawbone_gump.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Makefile.am
# End Source File
# Begin Source File

SOURCE=..\gumps\misc_buttons.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Newfile_gump.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Paperdoll_gump.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Paperdoll_gump_info.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Scroll_gump.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Sign_gump.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Slider_gump.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Spellbook_gump.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Stats_gump.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Text_button.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Text_gump.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\VideoOptions_gump.cc
# End Source File
# Begin Source File

SOURCE=..\gumps\Yesno_gump.cc
# End Source File
# End Group
# Begin Group "imagewin"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\imagewin\ibuf16.cc
# End Source File
# Begin Source File

SOURCE=..\imagewin\ibuf8.cc
# End Source File
# Begin Source File

SOURCE=..\imagewin\imagebuf.cc
# End Source File
# Begin Source File

SOURCE=..\imagewin\imagescl.cc
# End Source File
# Begin Source File

SOURCE=..\imagewin\imagewin.cc
# End Source File
# Begin Source File

SOURCE=..\imagewin\iwin8.cc
# End Source File
# Begin Source File

SOURCE=..\imagewin\Makefile.am
# End Source File
# Begin Source File

SOURCE=..\imagewin\savepcx.cc
# End Source File
# Begin Source File

SOURCE=..\imagewin\scale.cc

!IF  "$(CFG)" == "Exult - Win32 Release"

# PROP Exclude_From_Build 1
# PROP Ignore_Default_Tool 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug"

# PROP Exclude_From_Build 1
# PROP Ignore_Default_Tool 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Release Install"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1
# PROP Ignore_Default_Tool 1

!ELSEIF  "$(CFG)" == "Exult - Win32 Debug Install"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1
# PROP Ignore_Default_Tool 1

!ENDIF 

# End Source File
# End Group
# Begin Group "objs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\objs\animate.cc
# End Source File
# Begin Source File

SOURCE=..\objs\barge.cc
# End Source File
# Begin Source File

SOURCE=..\objs\chunks.cc
# End Source File
# Begin Source File

SOURCE=..\objs\chunkter.cc
# End Source File
# Begin Source File

SOURCE=..\objs\contain.cc
# End Source File
# Begin Source File

SOURCE=..\objs\egg.cc
# End Source File
# Begin Source File

SOURCE=..\objs\iregobjs.cc
# End Source File
# Begin Source File

SOURCE=..\objs\jawbone.cc
# End Source File
# Begin Source File

SOURCE=..\objs\Makefile.am
# End Source File
# Begin Source File

SOURCE=..\objs\mappatch.cc
# End Source File
# Begin Source File

SOURCE=..\objs\Objs.cc
# End Source File
# Begin Source File

SOURCE=..\objs\spellbook.cc
# End Source File
# Begin Source File

SOURCE=..\objs\virstone.cc
# End Source File
# End Group
# Begin Group "pathfinder"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\pathfinder\Astar.cc
# End Source File
# Begin Source File

SOURCE=..\pathfinder\Makefile.am
# End Source File
# Begin Source File

SOURCE=..\pathfinder\path.cc
# End Source File
# Begin Source File

SOURCE=..\pathfinder\PathFinder.cc
# End Source File
# Begin Source File

SOURCE=..\pathfinder\Zombie.cc
# End Source File
# End Group
# Begin Group "shapes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\shapes\bodies.cc
# End Source File
# Begin Source File

SOURCE=..\shapes\font.cc
# End Source File
# Begin Source File

SOURCE=..\shapes\fontvga.cc
# End Source File
# Begin Source File

SOURCE=..\shapes\items.cc
# End Source File
# Begin Source File

SOURCE=..\shapes\Makefile.am
# End Source File
# Begin Source File

SOURCE=..\shapes\monstinf.cc
# End Source File
# Begin Source File

SOURCE=..\shapes\shapeinf.cc
# End Source File
# Begin Source File

SOURCE=..\shapes\shapevga.cc
# End Source File
# Begin Source File

SOURCE=..\shapes\u7drag.cc
# End Source File
# Begin Source File

SOURCE=..\shapes\vgafile.cc
# End Source File
# End Group
# Begin Group "usecode"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\usecode\conversation.cc
# End Source File
# Begin Source File

SOURCE=..\usecode\intrinsics.cc
# End Source File
# Begin Source File

SOURCE=..\usecode\keyring.cc
# End Source File
# Begin Source File

SOURCE=..\usecode\Makefile.am
# End Source File
# Begin Source File

SOURCE=..\usecode\ucdisasm.cc
# End Source File
# Begin Source File

SOURCE=..\usecode\ucinternal.cc
# End Source File
# Begin Source File

SOURCE=..\usecode\ucmachine.cc
# End Source File
# Begin Source File

SOURCE=..\usecode\ucsched.cc
# End Source File
# Begin Source File

SOURCE=..\usecode\useval.cc
# End Source File
# End Group
# Begin Group "Server"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\server\objserial.cc
# End Source File
# Begin Source File

SOURCE=..\server\objserial.h
# End Source File
# Begin Source File

SOURCE=..\server\servemsg.cc
# End Source File
# Begin Source File

SOURCE=..\server\servemsg.h
# End Source File
# Begin Source File

SOURCE=..\server\server.cc
# End Source File
# Begin Source File

SOURCE=..\server\server.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\win32\exultico.rc
# ADD BASE RSC /l 0xc09 /i "\UC\exult\win32"
# ADD RSC /l 0xc09 /i "\UC\exult\win32" /i "..\win32"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Audio h"

# PROP Default_Filter ""
# Begin Group "Audio h unused"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\audio\midi_drivers\be_midi.h
# End Source File
# Begin Source File

SOURCE=..\audio\midi_drivers\forked_player.h
# End Source File
# Begin Source File

SOURCE=..\audio\midi_drivers\KMIDI.h
# End Source File
# Begin Source File

SOURCE=..\audio\midi_drivers\mac_midi.h
# End Source File
# Begin Source File

SOURCE=..\audio\midi_drivers\Timidity_binary.h
# End Source File
# Begin Source File

SOURCE=..\audio\midi_drivers\win_MCI.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\audio\Audio.h
# End Source File
# Begin Source File

SOURCE=..\audio\conv.h
# End Source File
# Begin Source File

SOURCE=..\audio\Midi.h
# End Source File
# Begin Source File

SOURCE=..\audio\Mixer.h
# End Source File
# Begin Source File

SOURCE=..\audio\pcb.h
# End Source File
# Begin Source File

SOURCE=..\audio\SDL_mapping.h
# End Source File
# Begin Source File

SOURCE=..\audio\soundtest.h
# End Source File
# Begin Source File

SOURCE=..\audio\midi_drivers\win_midiout.h
# End Source File
# Begin Source File

SOURCE=..\audio\xmidi.h
# End Source File
# End Group
# Begin Group "conf h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\conf\Configuration.h
# End Source File
# Begin Source File

SOURCE=..\conf\XMLEntity.h
# End Source File
# End Group
# Begin Group "Exult h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\acconfig.h
# End Source File
# Begin Source File

SOURCE=..\actions.h
# End Source File
# Begin Source File

SOURCE=..\actors.h
# End Source File
# Begin Source File

SOURCE=..\alpha_kludges.h
# End Source File
# Begin Source File

SOURCE=..\args.h
# End Source File
# Begin Source File

SOURCE=..\autoarray.h
# End Source File
# Begin Source File

SOURCE=..\bggame.h
# End Source File
# Begin Source File

SOURCE=..\browser.h
# End Source File
# Begin Source File

SOURCE=..\cheat.h
# End Source File
# Begin Source File

SOURCE=..\cheat_screen.h
# End Source File
# Begin Source File

SOURCE=..\combat.h
# End Source File
# Begin Source File

SOURCE=..\delobjs.h
# End Source File
# Begin Source File

SOURCE=..\dir.h
# End Source File
# Begin Source File

SOURCE=..\effects.h
# End Source File
# Begin Source File

SOURCE=..\exceptions.h
# End Source File
# Begin Source File

SOURCE=..\exult.h
# End Source File
# Begin Source File

SOURCE=..\exult_constants.h
# End Source File
# Begin Source File

SOURCE=..\exult_types.h
# End Source File
# Begin Source File

SOURCE=..\exultmenu.h
# End Source File
# Begin Source File

SOURCE=..\fnames.h
# End Source File
# Begin Source File

SOURCE=..\frameseq.h
# End Source File
# Begin Source File

SOURCE=..\game.h
# End Source File
# Begin Source File

SOURCE=..\gameclk.h
# End Source File
# Begin Source File

SOURCE=..\gamemap.h
# End Source File
# Begin Source File

SOURCE=..\gamewin.h
# End Source File
# Begin Source File

SOURCE=..\gamma.h
# End Source File
# Begin Source File

SOURCE=..\hash_utils.h
# End Source File
# Begin Source File

SOURCE=..\keyactions.h
# End Source File
# Begin Source File

SOURCE=..\keys.h
# End Source File
# Begin Source File

SOURCE=..\lists.h
# End Source File
# Begin Source File

SOURCE=..\menulist.h
# End Source File
# Begin Source File

SOURCE=..\mingw_kludges.h
# End Source File
# Begin Source File

SOURCE=..\monsters.h
# End Source File
# Begin Source File

SOURCE=..\mouse.h
# End Source File
# Begin Source File

SOURCE=..\npcnear.h
# End Source File
# Begin Source File

SOURCE=..\npctime.h
# End Source File
# Begin Source File

SOURCE=..\palette.h
# End Source File
# Begin Source File

SOURCE=..\paths.h
# End Source File
# Begin Source File

SOURCE=..\ready.h
# End Source File
# Begin Source File

SOURCE=..\rect.h
# End Source File
# Begin Source File

SOURCE=..\schedule.h
# End Source File
# Begin Source File

SOURCE=..\segfile.h
# End Source File
# Begin Source File

SOURCE=..\shapeid.h
# End Source File
# Begin Source File

SOURCE=..\sigame.h
# End Source File
# Begin Source File

SOURCE=..\tiles.h
# End Source File
# Begin Source File

SOURCE=..\tqueue.h
# End Source File
# Begin Source File

SOURCE=..\txtscroll.h
# End Source File
# Begin Source File

SOURCE=..\vec.h
# End Source File
# Begin Source File

SOURCE=..\version.h
# End Source File
# Begin Source File

SOURCE=..\xdrag.h
# End Source File
# End Group
# Begin Group "files h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\files\crc.h
# End Source File
# Begin Source File

SOURCE=..\files\databuf.h
# End Source File
# Begin Source File

SOURCE=..\files\Flat.h
# End Source File
# Begin Source File

SOURCE=..\files\Flex.h
# End Source File
# Begin Source File

SOURCE=..\files\IFF.h
# End Source File
# Begin Source File

SOURCE=..\files\listfiles.h
# End Source File
# Begin Source File

SOURCE=..\files\Table.h
# End Source File
# Begin Source File

SOURCE=..\files\U7file.h
# End Source File
# Begin Source File

SOURCE=..\files\utils.h
# End Source File
# Begin Source File

SOURCE=..\files\zip\zip_u7file.h
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "flic h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\flic\playfli.h
# End Source File
# End Group
# Begin Group "gumps h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\gumps\Actor_gump.h
# End Source File
# Begin Source File

SOURCE=..\gumps\AudioOptions_gump.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Book_gump.h
# End Source File
# Begin Source File

SOURCE=..\gumps\CombatStats_gump.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Enabled_button.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Face_button.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Face_stats.h
# End Source File
# Begin Source File

SOURCE=..\gumps\File_gump.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Gamemenu_gump.h
# End Source File
# Begin Source File

SOURCE=..\gumps\GameplayOptions_gump.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Gump.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Gump_button.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Gump_manager.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Gump_ToggleButton.h
# End Source File
# Begin Source File

SOURCE=..\gumps\gump_types.h
# End Source File
# Begin Source File

SOURCE=..\gumps\gump_utils.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Gump_widget.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Jawbone_gump.h
# End Source File
# Begin Source File

SOURCE=..\gumps\misc_buttons.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Modal_gump.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Newfile_gump.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Paperdoll_gump.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Scroll_gump.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Sign_gump.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Slider_gump.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Spellbook_gump.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Stats_gump.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Text_button.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Text_gump.h
# End Source File
# Begin Source File

SOURCE=..\gumps\VideoOptions_gump.h
# End Source File
# Begin Source File

SOURCE=..\gumps\Yesno_gump.h
# End Source File
# End Group
# Begin Group "imagewin h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\imagewin\ibuf16.h
# End Source File
# Begin Source File

SOURCE=..\imagewin\ibuf8.h
# End Source File
# Begin Source File

SOURCE=..\imagewin\imagebuf.h
# End Source File
# Begin Source File

SOURCE=..\imagewin\imagewin.h
# End Source File
# Begin Source File

SOURCE=..\imagewin\iwin8.h
# End Source File
# Begin Source File

SOURCE=..\imagewin\scale.h
# End Source File
# End Group
# Begin Group "objs h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\objs\animate.h
# End Source File
# Begin Source File

SOURCE=..\objs\barge.h
# End Source File
# Begin Source File

SOURCE=..\objs\chunks.h
# End Source File
# Begin Source File

SOURCE=..\objs\chunkter.h
# End Source File
# Begin Source File

SOURCE=..\objs\citerate.h
# End Source File
# Begin Source File

SOURCE=..\objs\contain.h
# End Source File
# Begin Source File

SOURCE=..\objs\egg.h
# End Source File
# Begin Source File

SOURCE=..\objs\flags.h
# End Source File
# Begin Source File

SOURCE=..\objs\iregobjs.h
# End Source File
# Begin Source File

SOURCE=..\objs\jawbone.h
# End Source File
# Begin Source File

SOURCE=..\objs\mappatch.h
# End Source File
# Begin Source File

SOURCE=..\shapes\monstinf.h
# End Source File
# Begin Source File

SOURCE=..\objs\objiter.h
# End Source File
# Begin Source File

SOURCE=..\objs\objlist.h
# End Source File
# Begin Source File

SOURCE=..\objs\objs.h
# End Source File
# Begin Source File

SOURCE=..\objs\ordinfo.h
# End Source File
# Begin Source File

SOURCE=..\objs\spellbook.h
# End Source File
# Begin Source File

SOURCE=..\objs\virstone.h
# End Source File
# End Group
# Begin Group "pathfinder h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\pathfinder\Astar.h
# End Source File
# Begin Source File

SOURCE=..\pathfinder\PathFinder.h
# End Source File
# Begin Source File

SOURCE=..\pathfinder\Zombie.h
# End Source File
# End Group
# Begin Group "shapes h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\shapes\bodies.h
# End Source File
# Begin Source File

SOURCE=..\shapes\font.h
# End Source File
# Begin Source File

SOURCE=..\shapes\fontvga.h
# End Source File
# Begin Source File

SOURCE=..\shapes\items.h
# End Source File
# Begin Source File

SOURCE=..\shapes\shapeinf.h
# End Source File
# Begin Source File

SOURCE=..\shapes\shapevga.h
# End Source File
# Begin Source File

SOURCE=..\shapes\u7drag.h
# End Source File
# Begin Source File

SOURCE=..\shapes\vgafile.h
# End Source File
# End Group
# Begin Group "usecode h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\usecode\bgintrinsics.h
# End Source File
# Begin Source File

SOURCE=..\usecode\conversation.h
# End Source File
# Begin Source File

SOURCE=..\usecode\keyring.h
# End Source File
# Begin Source File

SOURCE=..\usecode\siintrinsics.h
# End Source File
# Begin Source File

SOURCE=..\usecode\ucinternal.h
# End Source File
# Begin Source File

SOURCE=..\usecode\ucmachine.h
# End Source File
# Begin Source File

SOURCE=..\usecode\ucsched.h
# End Source File
# Begin Source File

SOURCE=..\usecode\ucscriptop.h
# End Source File
# Begin Source File

SOURCE=..\usecode\useval.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\data\exult_bg_flx.h
# End Source File
# Begin Source File

SOURCE=..\data\exult_flx.h
# End Source File
# Begin Source File

SOURCE=..\data\exult_si_flx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\win32\exult.ico
# End Source File
# End Group
# Begin Group "MSVC Kludges"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\find_nearby.h
# End Source File
# Begin Source File

SOURCE=.\msvc_kludges.cc
# ADD CPP /Yc"msvc_kludges.h"
# End Source File
# Begin Source File

SOURCE=.\msvc_kludges.h
# End Source File
# Begin Source File

SOURCE=.\unistd.h
# End Source File
# End Group
# Begin Group "Text Files"

# PROP Default_Filter ""
# Begin Group "Docs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\docs\art.txt
# End Source File
# Begin Source File

SOURCE=..\docs\bgitems.txt
# End Source File
# Begin Source File

SOURCE=..\docs\FLI.txt
# End Source File
# Begin Source File

SOURCE=..\docs\schedule.txt
# End Source File
# Begin Source File

SOURCE=..\docs\serpentsfx.txt
# End Source File
# Begin Source File

SOURCE=..\docs\siitems.txt
# End Source File
# Begin Source File

SOURCE=..\docs\u7combat.txt
# End Source File
# Begin Source File

SOURCE=..\docs\u7edit.txt
# End Source File
# Begin Source File

SOURCE=..\docs\u7eggs.txt
# End Source File
# Begin Source File

SOURCE=..\docs\u7itemtypes.txt
# End Source File
# Begin Source File

SOURCE=..\docs\u7monst.txt
# End Source File
# Begin Source File

SOURCE=..\docs\u7ready.txt
# End Source File
# Begin Source File

SOURCE=..\docs\u7tech.txt
# End Source File
# Begin Source File

SOURCE=..\docs\weapons.txt
# End Source File
# End Group
# Begin Source File

SOURCE=..\AUTHORS
# End Source File
# Begin Source File

SOURCE=..\ChangeLog
# End Source File
# Begin Source File

SOURCE=..\docs\excinematics.txt
# End Source File
# Begin Source File

SOURCE=..\FAQ
# End Source File
# Begin Source File

SOURCE=..\INSTALL
# End Source File
# Begin Source File

SOURCE=..\Makefile
# End Source File
# Begin Source File

SOURCE=..\data\Makefile.am
# End Source File
# Begin Source File

SOURCE=..\Makefile.be
# End Source File
# Begin Source File

SOURCE=..\Makefile.cygwin
# End Source File
# Begin Source File

SOURCE=..\Makefile.win32
# End Source File
# Begin Source File

SOURCE=..\message.txt
# End Source File
# Begin Source File

SOURCE=..\MidiInfo.txt
# End Source File
# Begin Source File

SOURCE=..\NEWS
# End Source File
# Begin Source File

SOURCE=..\README
# End Source File
# Begin Source File

SOURCE=..\README.1ST
# End Source File
# Begin Source File

SOURCE=..\README.axp
# End Source File
# Begin Source File

SOURCE=..\README.BeOS
# End Source File
# Begin Source File

SOURCE=..\README.MacOS
# End Source File
# Begin Source File

SOURCE=..\README.win32
# End Source File
# End Group
# End Target
# End Project
