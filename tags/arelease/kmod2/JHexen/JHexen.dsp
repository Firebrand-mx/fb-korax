# Microsoft Developer Studio Project File - Name="JHexen" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=JHexen - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "JHexen.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "JHexen.mak" CFG="JHexen - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "JHexen - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "JHexen - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "JHexen - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JHEXEN_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "NORANGECHECKING" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JHEXEN_EXPORTS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40b /d "NDEBUG"
# ADD RSC /l 0x40b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "JHexen - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JHEXEN_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JHEXEN_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40b /d "_DEBUG"
# ADD RSC /l 0x40b /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "JHexen - Win32 Release"
# Name "JHexen - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Play"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=.\P_acs.c
# End Source File
# Begin Source File

SOURCE=.\P_anim.c
# End Source File
# Begin Source File

SOURCE=.\P_ceilng.c
# End Source File
# Begin Source File

SOURCE=.\P_doors.c
# End Source File
# Begin Source File

SOURCE=.\P_enemy.c
# End Source File
# Begin Source File

SOURCE=.\P_floor.c
# End Source File
# Begin Source File

SOURCE=.\P_inter.c
# End Source File
# Begin Source File

SOURCE=.\P_lights.c
# End Source File
# Begin Source File

SOURCE=.\P_map.c
# End Source File
# Begin Source File

SOURCE=.\P_maputl.c
# End Source File
# Begin Source File

SOURCE=.\P_mobj.c
# End Source File
# Begin Source File

SOURCE=.\P_plats.c
# End Source File
# Begin Source File

SOURCE=.\P_pspr.c
# End Source File
# Begin Source File

SOURCE=.\P_setup.c
# End Source File
# Begin Source File

SOURCE=.\P_sight.c
# End Source File
# Begin Source File

SOURCE=.\P_spec.c
# End Source File
# Begin Source File

SOURCE=.\P_switch.c
# End Source File
# Begin Source File

SOURCE=.\P_telept.c
# End Source File
# Begin Source File

SOURCE=.\P_things.c
# End Source File
# Begin Source File

SOURCE=.\P_tick.c
# End Source File
# Begin Source File

SOURCE=.\P_user.c
# End Source File
# End Group
# Begin Group "Base-level"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=.\H2_Actn.c
# End Source File
# Begin Source File

SOURCE=.\H2_main.c
# End Source File
# Begin Source File

SOURCE=.\H2_net.c
# End Source File
# Begin Source File

SOURCE=.\HConsole.c
# End Source File
# Begin Source File

SOURCE=.\HRefresh.c
# End Source File
# End Group
# Begin Group "Menu"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Mn_menu.c
# End Source File
# Begin Source File

SOURCE=.\Mn_mplr.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\A_action.c
# End Source File
# Begin Source File

SOURCE=.\Am_map.c
# End Source File
# Begin Source File

SOURCE=.\Ct_chat.c
# End Source File
# Begin Source File

SOURCE=.\F_finale.c
# End Source File
# Begin Source File

SOURCE=.\G_game.c
# End Source File
# Begin Source File

SOURCE=.\In_lude.c
# End Source File
# Begin Source File

SOURCE=.\Info.c
# End Source File
# Begin Source File

SOURCE=.\JHexen.rc
# End Source File
# Begin Source File

SOURCE=.\m_bams.c
# End Source File
# Begin Source File

SOURCE=.\M_misc.c
# End Source File
# Begin Source File

SOURCE=.\mlo_info.c
# End Source File
# Begin Source File

SOURCE=.\Po_man.c
# End Source File
# Begin Source File

SOURCE=.\s_sound.c
# End Source File
# Begin Source File

SOURCE=.\Sb_bar.c
# End Source File
# Begin Source File

SOURCE=.\Sc_man.c
# End Source File
# Begin Source File

SOURCE=.\Sn_sonix.c
# End Source File
# Begin Source File

SOURCE=.\Sounds.c
# End Source File
# Begin Source File

SOURCE=.\Sv_save.c
# End Source File
# Begin Source File

SOURCE=.\Tables.c
# End Source File
# Begin Source File

SOURCE=.\Template.c

!IF  "$(CFG)" == "JHexen - Win32 Release"

!ELSEIF  "$(CFG)" == "JHexen - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\X_hair.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Am_data.h
# End Source File
# Begin Source File

SOURCE=.\Am_map.h
# End Source File
# Begin Source File

SOURCE=.\Ct_chat.h
# End Source File
# Begin Source File

SOURCE=.\Dstrings.h
# End Source File
# Begin Source File

SOURCE=.\g_demo.h
# End Source File
# Begin Source File

SOURCE=.\H2_Actn.h
# End Source File
# Begin Source File

SOURCE=.\H2_net.h
# End Source File
# Begin Source File

SOURCE=.\H2def.h
# End Source File
# Begin Source File

SOURCE=.\Info.h
# End Source File
# Begin Source File

SOURCE=.\Journal.h
# End Source File
# Begin Source File

SOURCE=.\m_bams.h
# End Source File
# Begin Source File

SOURCE=.\Mn_def.h
# End Source File
# Begin Source File

SOURCE=.\P_local.h
# End Source File
# Begin Source File

SOURCE=.\P_spec.h
# End Source File
# Begin Source File

SOURCE=.\R_local.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\Settings.h
# End Source File
# Begin Source File

SOURCE=.\Sounds.h
# End Source File
# Begin Source File

SOURCE=.\Soundst.h
# End Source File
# Begin Source File

SOURCE=.\Textdefs.h
# End Source File
# Begin Source File

SOURCE=.\Xddefs.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Hexen.ico
# End Source File
# Begin Source File

SOURCE=.\JHexen.ico
# End Source File
# End Group
# End Target
# End Project
