# Microsoft Developer Studio Project File - Name="shp2pcx" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=shp2pcx - Win32 Debug Install
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "shp2pcx.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "shp2pcx.mak" CFG="shp2pcx - Win32 Debug Install"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "shp2pcx - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "shp2pcx - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "shp2pcx - Win32 Release Install" (based on "Win32 (x86) Console Application")
!MESSAGE "shp2pcx - Win32 Debug Install" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "shp2pcx - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "shp2pcx___Win32_Release"
# PROP BASE Intermediate_Dir "shp2pcx___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\tools"
# PROP Intermediate_Dir "shp2pcx_release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O1 /I "..\\" /I "..\Gumps" /I "..\conf" /I "..\files" /I "..\filc" /I "..\imagewin" /I "..\objs" /I "..\pathfinder" /I "..\shapes" /I "..\usecode" /I "..\MSVCStuff" /I "..\Audio" /I "..\Audio\MIDI_drivers" /I "..\Headers" /FI"msvc_kludges.h" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /TP /c
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"shp2pcx_release/shp2pcx.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /machine:I386

!ELSEIF  "$(CFG)" == "shp2pcx - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "shp2pcx___Win32_Debug"
# PROP BASE Intermediate_Dir "shp2pcx___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\tools"
# PROP Intermediate_Dir "shp2pcx_debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\\" /I "..\Gumps" /I "..\conf" /I "..\files" /I "..\filc" /I "..\imagewin" /I "..\objs" /I "..\pathfinder" /I "..\shapes" /I "..\usecode" /I "..\MSVCStuff" /I "..\Audio" /I "..\Audio\MIDI_drivers" /I "..\Headers" /FI"msvc_kludges.h" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /TP /c
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"shp2pcx_debug/shp2pcx.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /incremental:no

!ELSEIF  "$(CFG)" == "shp2pcx - Win32 Release Install"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "shp2pcx___Win32_Release_Install"
# PROP BASE Intermediate_Dir "shp2pcx___Win32_Release_Install"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\tools"
# PROP Intermediate_Dir "shp2pcx_release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\\" /I "..\Gumps" /I "..\conf" /I "..\files" /I "..\filc" /I "..\imagewin" /I "..\objs" /I "..\pathfinder" /I "..\shapes" /I "..\usecode" /I "..\MSVCStuff" /I "..\SDL-1.2.0\include" /I "..\Audio" /I "..\Audio\MIDI_drivers" /FI"msvc_kludges.h" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /TP /c
# ADD CPP /nologo /W3 /GX /O1 /I "..\\" /I "..\Gumps" /I "..\conf" /I "..\files" /I "..\filc" /I "..\imagewin" /I "..\objs" /I "..\pathfinder" /I "..\shapes" /I "..\usecode" /I "..\MSVCStuff" /I "..\Audio" /I "..\Audio\MIDI_drivers" /I "..\Headers" /FI"msvc_kludges.h" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /TP /c
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"shp2pcx_release/shp2pcx.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /machine:I386
# Begin Custom Build - Installing $(InputName).exe
InputPath=\UC\exult\tools\shp2pcx.exe
InputName=shp2pcx
SOURCE="$(InputPath)"

"$(EXULT_INSTALL_PATH)\tools\$(InputName).exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	if not exist "$(EXULT_INSTALL_PATH)\tools" md "$(EXULT_INSTALL_PATH)\tools" 
	copy $(InputPath) "$(EXULT_INSTALL_PATH)\tools" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "shp2pcx - Win32 Debug Install"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "shp2pcx___Win32_Debug_Install"
# PROP BASE Intermediate_Dir "shp2pcx___Win32_Debug_Install"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\tools"
# PROP Intermediate_Dir "shp2pcx_debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\\" /I "..\Gumps" /I "..\conf" /I "..\files" /I "..\filc" /I "..\imagewin" /I "..\objs" /I "..\pathfinder" /I "..\shapes" /I "..\usecode" /I "..\MSVCStuff" /I "..\SDL-1.2.0\include" /I "..\Audio" /I "..\Audio\MIDI_drivers" /FI"msvc_kludges.h" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /TP /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\\" /I "..\Gumps" /I "..\conf" /I "..\files" /I "..\filc" /I "..\imagewin" /I "..\objs" /I "..\pathfinder" /I "..\shapes" /I "..\usecode" /I "..\MSVCStuff" /I "..\Audio" /I "..\Audio\MIDI_drivers" /I "..\Headers" /FI"msvc_kludges.h" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /TP /c
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"shp2pcx_debug/shp2pcx.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /incremental:no
# Begin Custom Build - Installing $(InputName).exe
InputPath=\UC\exult\tools\shp2pcx.exe
InputName=shp2pcx
SOURCE="$(InputPath)"

"$(EXULT_INSTALL_PATH)\tools\$(InputName).exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	if not exist "$(EXULT_INSTALL_PATH)\tools" md "$(EXULT_INSTALL_PATH)\tools" 
	copy $(InputPath) "$(EXULT_INSTALL_PATH)\tools" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "shp2pcx - Win32 Release"
# Name "shp2pcx - Win32 Debug"
# Name "shp2pcx - Win32 Release Install"
# Name "shp2pcx - Win32 Debug Install"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\tools\shp2pcx.cc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\exult_types.h
# End Source File
# Begin Source File

SOURCE=.\msvc_kludges.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
