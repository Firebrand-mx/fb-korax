# Microsoft Developer Studio Project File - Name="ScatteredEvil" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ScatteredEvil - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ScatteredEvil.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ScatteredEvil.mak" CFG="ScatteredEvil - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ScatteredEvil - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ScatteredEvil - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ScatteredEvil - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SCATTEREDEVIL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SCATTEREDEVIL_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x426 /d "NDEBUG"
# ADD RSC /l 0x426 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "ScatteredEvil - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SCATTEREDEVIL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SCATTEREDEVIL_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x426 /d "_DEBUG"
# ADD RSC /l 0x426 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "ScatteredEvil - Win32 Release"
# Name "ScatteredEvil - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\a_action.cpp
# End Source File
# Begin Source File

SOURCE=.\am_map.cpp
# End Source File
# Begin Source File

SOURCE=.\ct_chat.cpp
# End Source File
# Begin Source File

SOURCE=.\f_finale.cpp
# End Source File
# Begin Source File

SOURCE=.\g_game.cpp
# End Source File
# Begin Source File

SOURCE=.\h2_actn.cpp
# End Source File
# Begin Source File

SOURCE=.\h2_main.cpp
# End Source File
# Begin Source File

SOURCE=.\h2_net.cpp
# End Source File
# Begin Source File

SOURCE=.\hconsole.cpp
# End Source File
# Begin Source File

SOURCE=.\hrefresh.cpp
# End Source File
# Begin Source File

SOURCE=.\in_lude.cpp
# End Source File
# Begin Source File

SOURCE=.\info.cpp
# End Source File
# Begin Source File

SOURCE=.\KCanvas.cpp
# End Source File
# Begin Source File

SOURCE=.\KGC.cpp
# End Source File
# Begin Source File

SOURCE=.\KObject.cpp

!IF  "$(CFG)" == "ScatteredEvil - Win32 Release"

# ADD CPP /w /W0

!ELSEIF  "$(CFG)" == "ScatteredEvil - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\KRootWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\KWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\m_bams.cpp
# End Source File
# Begin Source File

SOURCE=.\m_misc.cpp
# End Source File
# Begin Source File

SOURCE=.\mlo_info.cpp
# End Source File
# Begin Source File

SOURCE=.\mn_menu.cpp
# End Source File
# Begin Source File

SOURCE=.\p_acs.cpp
# End Source File
# Begin Source File

SOURCE=.\p_anim.cpp
# End Source File
# Begin Source File

SOURCE=.\p_ceilng.cpp
# End Source File
# Begin Source File

SOURCE=.\p_doors.cpp
# End Source File
# Begin Source File

SOURCE=.\p_enemy.cpp
# End Source File
# Begin Source File

SOURCE=.\p_floor.cpp
# End Source File
# Begin Source File

SOURCE=.\p_inter.cpp
# End Source File
# Begin Source File

SOURCE=.\p_lights.cpp
# End Source File
# Begin Source File

SOURCE=.\p_map.cpp
# End Source File
# Begin Source File

SOURCE=.\p_maputl.cpp
# End Source File
# Begin Source File

SOURCE=.\p_mobj.cpp
# End Source File
# Begin Source File

SOURCE=.\p_npc.cpp
# End Source File
# Begin Source File

SOURCE=.\p_plats.cpp
# End Source File
# Begin Source File

SOURCE=.\p_pspr.cpp
# End Source File
# Begin Source File

SOURCE=.\p_setup.cpp
# End Source File
# Begin Source File

SOURCE=.\p_sight.cpp
# End Source File
# Begin Source File

SOURCE=.\p_spec.cpp
# End Source File
# Begin Source File

SOURCE=.\p_switch.cpp
# End Source File
# Begin Source File

SOURCE=.\p_telept.cpp
# End Source File
# Begin Source File

SOURCE=.\p_things.cpp
# End Source File
# Begin Source File

SOURCE=.\p_tick.cpp
# End Source File
# Begin Source File

SOURCE=.\p_user.cpp
# End Source File
# Begin Source File

SOURCE=.\po_man.cpp
# End Source File
# Begin Source File

SOURCE=.\s_sound.cpp
# End Source File
# Begin Source File

SOURCE=.\sb_bar.cpp
# End Source File
# Begin Source File

SOURCE=.\sc_man.cpp
# End Source File
# Begin Source File

SOURCE=.\sn_sonix.cpp
# End Source File
# Begin Source File

SOURCE=.\sounds.cpp
# End Source File
# Begin Source File

SOURCE=.\sv_save.cpp
# End Source File
# Begin Source File

SOURCE=.\tables.cpp
# End Source File
# Begin Source File

SOURCE=.\template.cpp
# End Source File
# Begin Source File

SOURCE=.\x_hair.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\am_data.h
# End Source File
# Begin Source File

SOURCE=.\am_map.h
# End Source File
# Begin Source File

SOURCE=.\ct_chat.h
# End Source File
# Begin Source File

SOURCE=.\drcoord.h
# End Source File
# Begin Source File

SOURCE=.\h2_actn.h
# End Source File
# Begin Source File

SOURCE=.\h2_net.h
# End Source File
# Begin Source File

SOURCE=.\h2def.h
# End Source File
# Begin Source File

SOURCE=.\i_header.h
# End Source File
# Begin Source File

SOURCE=.\info.h
# End Source File
# Begin Source File

SOURCE=.\journal.h
# End Source File
# Begin Source File

SOURCE=.\KCanvas.h
# End Source File
# Begin Source File

SOURCE=.\KGC.h
# End Source File
# Begin Source File

SOURCE=.\KObject.h
# End Source File
# Begin Source File

SOURCE=.\KRootWindow.h
# End Source File
# Begin Source File

SOURCE=.\KWindow.h
# End Source File
# Begin Source File

SOURCE=.\m_bams.h
# End Source File
# Begin Source File

SOURCE=.\mn_char.h
# End Source File
# Begin Source File

SOURCE=.\mn_class.h
# End Source File
# Begin Source File

SOURCE=.\mn_controls.h
# End Source File
# Begin Source File

SOURCE=.\mn_def.h
# End Source File
# Begin Source File

SOURCE=.\mn_effects.h
# End Source File
# Begin Source File

SOURCE=.\mn_gameplay.h
# End Source File
# Begin Source File

SOURCE=.\mn_gfx.h
# End Source File
# Begin Source File

SOURCE=.\mn_info.h
# End Source File
# Begin Source File

SOURCE=.\mn_joy.h
# End Source File
# Begin Source File

SOURCE=.\mn_load.h
# End Source File
# Begin Source File

SOURCE=.\mn_main.h
# End Source File
# Begin Source File

SOURCE=.\mn_mouse.h
# End Source File
# Begin Source File

SOURCE=.\mn_options.h
# End Source File
# Begin Source File

SOURCE=.\mn_resolution.h
# End Source File
# Begin Source File

SOURCE=.\mn_save.h
# End Source File
# Begin Source File

SOURCE=.\mn_skill.h
# End Source File
# Begin Source File

SOURCE=.\mn_sound.h
# End Source File
# Begin Source File

SOURCE=.\p_local.h
# End Source File
# Begin Source File

SOURCE=.\p_spec.h
# End Source File
# Begin Source File

SOURCE=.\r_local.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\settings.h
# End Source File
# Begin Source File

SOURCE=.\sounds.h
# End Source File
# Begin Source File

SOURCE=.\soundst.h
# End Source File
# Begin Source File

SOURCE=.\textdefs.h
# End Source File
# Begin Source File

SOURCE=.\xddefs.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\hexen.ico
# End Source File
# Begin Source File

SOURCE=.\jhexen.ico
# End Source File
# Begin Source File

SOURCE=.\ScatteredEvil.rc
# End Source File
# End Group
# Begin Group "Config Files"

# PROP Default_Filter "cfg"
# Begin Source File

SOURCE=..\autoexec.cfg
# End Source File
# End Group
# End Target
# End Project
