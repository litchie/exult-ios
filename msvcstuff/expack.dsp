# Microsoft Developer Studio Project File - Name="expack" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=expack - Win32 Debug Install
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "expack.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "expack.mak" CFG="expack - Win32 Debug Install"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "expack - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "expack - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "expack - Win32 Release Install" (based on "Win32 (x86) Console Application")
!MESSAGE "expack - Win32 Debug Install" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "expack - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "expack___Win32_Release"
# PROP BASE Intermediate_Dir "expack___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\tools"
# PROP Intermediate_Dir "expack_release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O1 /I "..\\" /I "..\Gumps" /I "..\conf" /I "..\files" /I "..\filc" /I "..\imagewin" /I "..\objs" /I "..\pathfinder" /I "..\shapes" /I "..\usecode" /I "..\MSVCStuff" /I "..\Audio" /I "..\Audio\MIDI_drivers" /I "..\Headers" /FI"msvc_kludges.h" /D "NDEBUG" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /D "USING_PRECOMPILED_HEADER" /Yu"msvc_kludges.h" /FD /TP /c
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"expack_release/expack.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /machine:I386
# Begin Custom Build - Creating buildflx.bat
InputPath=\UC\exult\tools\expack.exe
SOURCE="$(InputPath)"

"buildflx.bat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo cd "%%1" > buildflx.bat 
	echo $(InputPath) -i flx.in >> buildflx.bat 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "expack - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "expack___Win32_Debug"
# PROP BASE Intermediate_Dir "expack___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\tools"
# PROP Intermediate_Dir "expack_debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\\" /I "..\Gumps" /I "..\conf" /I "..\files" /I "..\filc" /I "..\imagewin" /I "..\objs" /I "..\pathfinder" /I "..\shapes" /I "..\usecode" /I "..\MSVCStuff" /I "..\Audio" /I "..\Audio\MIDI_drivers" /I "..\Headers" /FI"msvc_kludges.h" /D "_DEBUG" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /D "USING_PRECOMPILED_HEADER" /Yu"msvc_kludges.h" /FD /GZ /TP /c
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"expack_debug/expack.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /incremental:no
# Begin Custom Build - Creating buildflx.bat
InputPath=\UC\exult\tools\expack.exe
SOURCE="$(InputPath)"

"buildflx.bat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo cd "%%1" > buildflx.bat 
	echo $(InputPath) -i flx.in >> buildflx.bat 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "expack - Win32 Release Install"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "expack___Win32_Release_Install"
# PROP BASE Intermediate_Dir "expack___Win32_Release_Install"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\tools"
# PROP Intermediate_Dir "expack_release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "..\\" /I "..\Gumps" /I "..\conf" /I "..\files" /I "..\filc" /I "..\imagewin" /I "..\objs" /I "..\pathfinder" /I "..\shapes" /I "..\usecode" /I "..\MSVCStuff" /I "..\SDL-1.2.0\include" /I "..\Audio" /I "..\Audio\MIDI_drivers" /FI"msvc_kludges.h" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"msvc_kludges.h" /FD /TP /c
# ADD CPP /nologo /W3 /GX /O1 /I "..\\" /I "..\Gumps" /I "..\conf" /I "..\files" /I "..\filc" /I "..\imagewin" /I "..\objs" /I "..\pathfinder" /I "..\shapes" /I "..\usecode" /I "..\MSVCStuff" /I "..\Audio" /I "..\Audio\MIDI_drivers" /I "..\Headers" /FI"msvc_kludges.h" /D "NDEBUG" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /D "USING_PRECOMPILED_HEADER" /Yu"msvc_kludges.h" /FD /TP /c
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"expack_release/expack.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /machine:I386
# Begin Custom Build - Installing $(InputName).exe and Creating buildflx_inst.bat
InputPath=\UC\exult\tools\expack.exe
InputName=expack
SOURCE="$(InputPath)"

BuildCmds= \
	echo cd "%%1" > buildflx_inst.bat \
	echo $(InputPath) -i flx.in >> buildflx_inst.bat \
	echo @if not exist "$(EXULT_INSTALL_PATH)\%%4" md "$(EXULT_INSTALL_PATH)\%%4" >> buildflx_inst.bat \
	echo cd "%%2" >> buildflx_inst.bat \
	echo copy "%%3" "$(EXULT_INSTALL_PATH)\%%4" >> buildflx_inst.bat \
	if not exist "$(EXULT_INSTALL_PATH)\tools" md "$(EXULT_INSTALL_PATH)\tools" \
	copy $(InputPath) "$(EXULT_INSTALL_PATH)\tools" \
	

"buildflx_inst.bat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(EXULT_INSTALL_PATH)\tools\$(InputName).exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "expack - Win32 Debug Install"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "expack___Win32_Debug_Install"
# PROP BASE Intermediate_Dir "expack___Win32_Debug_Install"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\tools"
# PROP Intermediate_Dir "expack_debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\\" /I "..\Gumps" /I "..\conf" /I "..\files" /I "..\filc" /I "..\imagewin" /I "..\objs" /I "..\pathfinder" /I "..\shapes" /I "..\usecode" /I "..\MSVCStuff" /I "..\SDL-1.2.0\include" /I "..\Audio" /I "..\Audio\MIDI_drivers" /FI"msvc_kludges.h" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"msvc_kludges.h" /FD /GZ /TP /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\\" /I "..\Gumps" /I "..\conf" /I "..\files" /I "..\filc" /I "..\imagewin" /I "..\objs" /I "..\pathfinder" /I "..\shapes" /I "..\usecode" /I "..\MSVCStuff" /I "..\Audio" /I "..\Audio\MIDI_drivers" /I "..\Headers" /FI"msvc_kludges.h" /D "_DEBUG" /D "_CONSOLE" /D "WIN32" /D "_MBCS" /D "USING_PRECOMPILED_HEADER" /Yu"msvc_kludges.h" /FD /GZ /TP /c
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"expack_debug/expack.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /incremental:no
# Begin Custom Build - Installing $(InputName).exe and Creating buildflx_inst.bat
InputPath=\UC\exult\tools\expack.exe
InputName=expack
SOURCE="$(InputPath)"

BuildCmds= \
	echo cd "%%1" > buildflx_inst.bat \
	echo $(InputPath) -i flx.in >> buildflx_inst.bat \
	echo @if not exist "$(EXULT_INSTALL_PATH)\%%4" md "$(EXULT_INSTALL_PATH)\%%4" >> buildflx_inst.bat \
	echo cd "%%2" >> buildflx_inst.bat \
	echo copy "%%3" "$(EXULT_INSTALL_PATH)\%%4" >> buildflx_inst.bat \
	if not exist "$(EXULT_INSTALL_PATH)\tools" md "$(EXULT_INSTALL_PATH)\tools" \
	copy $(InputPath) "$(EXULT_INSTALL_PATH)\tools" \
	

"buildflx_inst.bat" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(EXULT_INSTALL_PATH)\tools\$(InputName).exe" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# Begin Target

# Name "expack - Win32 Release"
# Name "expack - Win32 Debug"
# Name "expack - Win32 Release Install"
# Name "expack - Win32 Debug Install"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\files\crc.cc
# End Source File
# Begin Source File

SOURCE=..\tools\expack.cc
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

SOURCE=.\msvc_kludges.cc
# ADD CPP /Yc"msvc_kludges.h"
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
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\files\crc.h
# End Source File
# Begin Source File

SOURCE=..\files\databuf.h
# End Source File
# Begin Source File

SOURCE=..\exceptions.h
# End Source File
# Begin Source File

SOURCE=..\exult_types.h
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

SOURCE=.\msvc_kludges.h
# End Source File
# Begin Source File

SOURCE=..\files\Table.h
# End Source File
# Begin Source File

SOURCE=..\files\U7file.h
# End Source File
# Begin Source File

SOURCE=.\unistd.h
# End Source File
# Begin Source File

SOURCE=..\files\utils.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
