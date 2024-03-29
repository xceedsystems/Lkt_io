# Microsoft Developer Studio Project File - Name="Runtime" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Runtime - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Runtime.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Runtime.mak" CFG="Runtime - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Runtime - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Runtime - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Runtime - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /X /I "..\inc" /I "..\..\inc" /I "..\..\intime\inc" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_WIN32" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\inc" /i "..\..\inc" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 RtFlat.lib DcFlat.lib CsFlat.lib cstart32.obj rt.lib ciff3m.lib /nologo /version:21076.20052 /stack:0x100000,0x2000 /subsystem:console /map /debug /debugtype:both /machine:I386 /nodefaultlib /out:"\Program Files\VLC\Driver 6_X\Bin\Jobs\LKT_IO.rt3" /pdbtype:sept /libpath:"..\..\lib" /libpath:"..\..\intime\lib" /heap:0x4000000,0x4000
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Runtime - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /X /I "..\inc" /I "..\..\inc" /I "..\..\intime\inc" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_WIN32" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\inc" /i "..\..\inc" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 RtFlat.lib DcFlat.lib CsFlat.lib cstart32.obj rt.lib ciff3m.lib /nologo /version:21076.20052 /stack:0x100000,0x2000 /subsystem:console /pdb:none /debug /machine:I386 /nodefaultlib /out:"\Debug\LKT_IO.rta" /libpath:"..\..\lib" /libpath:"..\..\intime\lib" /heap:0x4000000,0x4000

!ENDIF 

# Begin Target

# Name "Runtime - Win32 Release"
# Name "Runtime - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\auxrut.c
# End Source File
# Begin Source File

SOURCE=.\Card.c
# End Source File
# Begin Source File

SOURCE=.\dmcgalil.c
# End Source File
# Begin Source File

SOURCE=.\LKT_IO.c
# End Source File
# Begin Source File

SOURCE=.\task.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\auxrut.h
# End Source File
# Begin Source File

SOURCE=.\Card.h
# End Source File
# Begin Source File

SOURCE=.\dmcgalil.h
# End Source File
# Begin Source File

SOURCE=.\LKT_IO.h
# End Source File
# Begin Source File

SOURCE=.\STDAFX.H
# End Source File
# Begin Source File

SOURCE=.\task.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\VERSION.RC
# End Source File
# End Group
# Begin Group "Local Inc"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=..\INC\Driver.h
# End Source File
# Begin Source File

SOURCE=..\INC\Errors.h
# End Source File
# Begin Source File

SOURCE=..\INC\Version.h
# End Source File
# End Group
# End Target
# End Project
