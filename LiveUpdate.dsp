# Microsoft Developer Studio Project File - Name="LiveUpdate" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=LiveUpdate - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "LiveUpdate.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "LiveUpdate.mak" CFG="LiveUpdate - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "LiveUpdate - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "LiveUpdate - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "LiveUpdate - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "LiveUpdate - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "LiveUpdate - Win32 Release"
# Name "LiveUpdate - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\LiveUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\LiveUpdate.rc
# End Source File
# Begin Source File

SOURCE=.\LiveUpdateDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\LiveUpdate.h
# End Source File
# Begin Source File

SOURCE=.\LiveUpdateDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\7za.exe.bin
# End Source File
# Begin Source File

SOURCE=.\res\file1.bin
# End Source File
# Begin Source File

SOURCE=.\res\left.bmp
# End Source File
# Begin Source File

SOURCE=.\res\left.jpg
# End Source File
# Begin Source File

SOURCE=.\res\LiveUpdate.ico
# End Source File
# Begin Source File

SOURCE=.\res\LiveUpdate.rc2
# End Source File
# Begin Source File

SOURCE=.\res\self_bat.bin
# End Source File
# Begin Source File

SOURCE=.\res\Selfbat.bin
# End Source File
# End Group
# Begin Group "Share"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Share\DownFile.cpp
# End Source File
# Begin Source File

SOURCE=..\Share\DownFile.h
# End Source File
# Begin Source File

SOURCE=..\Share\EMFC.cpp
# End Source File
# Begin Source File

SOURCE=..\Share\EMFC.h
# End Source File
# Begin Source File

SOURCE=..\Share\HyperLink.cpp
# End Source File
# Begin Source File

SOURCE=..\Share\HyperLink.h
# End Source File
# Begin Source File

SOURCE=..\Share\IniFile.cpp
# End Source File
# Begin Source File

SOURCE=..\Share\IniFile.h
# End Source File
# Begin Source File

SOURCE=..\Share\ListCtrlEx.cpp
# End Source File
# Begin Source File

SOURCE=..\Share\ListCtrlEx.h
# End Source File
# Begin Source File

SOURCE=..\Share\WND.cpp
# End Source File
# Begin Source File

SOURCE=..\Share\WND.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ChangeLog.txt
# End Source File
# Begin Source File

SOURCE=.\LiveUpdate.txt
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\res\Self.bat.txt
# End Source File
# Begin Source File

SOURCE=.\SelfUpdate.bat.txt
# End Source File
# End Target
# End Project
