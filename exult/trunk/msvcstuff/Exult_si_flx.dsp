# Microsoft Developer Studio Project File - Name="Exult_si_flx" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=Exult_si_flx - Win32 Debug Install
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Exult_si_flx.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Exult_si_flx.mak" CFG="Exult_si_flx - Win32 Debug Install"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Exult_si_flx - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "Exult_si_flx - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE "Exult_si_flx - Win32 Release Install" (based on "Win32 (x86) External Target")
!MESSAGE "Exult_si_flx - Win32 Debug Install" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "Exult_si_flx - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Exult_si_flx___Win32_Release"
# PROP BASE Intermediate_Dir "Exult_si_flx___Win32_Release"
# PROP BASE Cmd_Line "buildflx.bat ..\data\si"
# PROP BASE Rebuild_Opt ""
# PROP BASE Target_File "..\data\exult_si.flx"
# PROP BASE Bsc_Name ""
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Exult_si_flx___Win32_Release"
# PROP Intermediate_Dir "Exult_si_flx___Win32_Release"
# PROP Cmd_Line "buildflx.bat ..\data\si"
# PROP Rebuild_Opt ""
# PROP Target_File "..\data\exult_si_flx.h"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "Exult_si_flx - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Exult_si_flx___Win32_Debug0"
# PROP BASE Intermediate_Dir "Exult_si_flx___Win32_Debug0"
# PROP BASE Cmd_Line "buildflx.bat ..\data\si"
# PROP BASE Rebuild_Opt ""
# PROP BASE Target_File "..\data\exult_si.flx"
# PROP BASE Bsc_Name ""
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Exult_si_flx___Win32_Debug0"
# PROP Intermediate_Dir "Exult_si_flx___Win32_Debug0"
# PROP Cmd_Line "buildflx.bat ..\data\si"
# PROP Rebuild_Opt ""
# PROP Target_File "..\data\exult_si_flx.h"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "Exult_si_flx - Win32 Release Install"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Release Install"
# PROP BASE Intermediate_Dir "Release Install"
# PROP BASE Cmd_Line "buildflx.bat ..\data\si"
# PROP BASE Rebuild_Opt ""
# PROP BASE Target_File "..\data\exult_si.flx"
# PROP BASE Bsc_Name ""
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Release Install"
# PROP Intermediate_Dir "Release Install"
# PROP Cmd_Line "buildflx_inst.bat ..\data\si .. ..\data\exult_si.flx data"
# PROP Rebuild_Opt ""
# PROP Target_File "..\data\exult_si_flx.h"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "Exult_si_flx - Win32 Debug Install"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug Install"
# PROP BASE Intermediate_Dir "Debug Install"
# PROP BASE Cmd_Line "buildflx.bat ..\data\si"
# PROP BASE Rebuild_Opt ""
# PROP BASE Target_File "..\data\exult_si.flx"
# PROP BASE Bsc_Name ""
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug Install"
# PROP Intermediate_Dir "Debug Install"
# PROP Cmd_Line "buildflx_inst.bat ..\data\si .. ..\data\exult_si.flx data"
# PROP Rebuild_Opt ""
# PROP Target_File "..\data\exult_si_flx.h"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "Exult_si_flx - Win32 Release"
# Name "Exult_si_flx - Win32 Debug"
# Name "Exult_si_flx - Win32 Release Install"
# Name "Exult_si_flx - Win32 Debug Install"

!IF  "$(CFG)" == "Exult_si_flx - Win32 Release"

!ELSEIF  "$(CFG)" == "Exult_si_flx - Win32 Debug"

!ELSEIF  "$(CFG)" == "Exult_si_flx - Win32 Release Install"

!ELSEIF  "$(CFG)" == "Exult_si_flx - Win32 Debug Install"

!ENDIF 

# Begin Group "Shapes"

# PROP Default_Filter "shp"
# Begin Source File

SOURCE=..\data\si\SImap.shp
# PROP BASE Intermediate_Dir "."
# PROP Intermediate_Dir "."
# End Source File
# End Group
# Begin Group "Misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\data\si\defaultkeys.txt
# PROP BASE Intermediate_Dir "."
# PROP Intermediate_Dir "."
# End Source File
# End Group
# Begin Source File

SOURCE=..\data\si\flx.in
# PROP BASE Intermediate_Dir "."
# PROP Intermediate_Dir "."
# End Source File
# End Target
# End Project
