# Microsoft Developer Studio Project File - Name="exconfig" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=exconfig - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "exconfig.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "exconfig.mak" CFG="exconfig - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "exconfig - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "exconfig - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "exconfig - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "."
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EXCONFIG_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O1 /I "..\..\pathfinder" /I "..\\" /I "..\..\\" /I "..\..\conf" /I "..\..\files" /I "..\..\filc" /I "..\..\imagewin" /I "..\..\objs" /I "..\..\shapes" /I "..\..\usecode" /I "..\..\Audio" /I "..\..\Audio\MIDI_drivers" /I "..\..\data" /I "..\..\headers" /FI"StdAfx.h" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EXCONFIG_EXPORTS" /Yu"stdafx.h" /FD /TP /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Release/exconfig.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /version:0.98 /dll /incremental:yes /machine:I386

!ELSEIF  "$(CFG)" == "exconfig - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "."
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EXCONFIG_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\Gumps" /I "..\\" /I "..\..\\" /I "..\..\conf" /I "..\..\files" /I "..\..\filc" /I "..\..\imagewin" /I "..\..\objs" /I "..\..\shapes" /I "..\..\usecode" /I "..\..\Audio" /I "..\..\Audio\MIDI_drivers" /I "..\..\data" /I "..\..\headers" /FI"StdAfx.h" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EXCONFIG_EXPORTS" /FR /Yu"stdafx.h" /FD /TP /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Debug\exconfig.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /version:0.98 /dll /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /incremental:no

!ENDIF 

# Begin Target

# Name "exconfig - Win32 Release"
# Name "exconfig - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\conf\Configuration.cc
# End Source File
# Begin Source File

SOURCE=.\exconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\exconfig.def
# End Source File
# Begin Source File

SOURCE=.\exconfig.rc
# End Source File
# Begin Source File

SOURCE=..\..\files\Flat.cc
# End Source File
# Begin Source File

SOURCE=..\..\files\Flex.cc
# End Source File
# Begin Source File

SOURCE=..\..\files\IFF.cc
# End Source File
# Begin Source File

SOURCE=..\..\files\listfiles.cc
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\..\files\Table.cc
# End Source File
# Begin Source File

SOURCE=..\..\files\U7file.cc
# End Source File
# Begin Source File

SOURCE=..\..\files\utils.cc
# End Source File
# Begin Source File

SOURCE=..\..\conf\XMLEntity.cc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\conf\Configuration.h
# End Source File
# Begin Source File

SOURCE=..\..\files\databuf.h
# End Source File
# Begin Source File

SOURCE=.\exconfig.h
# End Source File
# Begin Source File

SOURCE=..\..\files\Flat.h
# End Source File
# Begin Source File

SOURCE=..\..\files\Flex.h
# End Source File
# Begin Source File

SOURCE=..\..\files\IFF.h
# End Source File
# Begin Source File

SOURCE=..\..\files\listfiles.h
# End Source File
# Begin Source File

SOURCE=..\msvc_kludges.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\..\files\Table.h
# End Source File
# Begin Source File

SOURCE=..\..\files\U7file.h
# End Source File
# Begin Source File

SOURCE=..\..\conf\XMLEntity.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
