# Microsoft Developer Studio Project File - Name="ScatteredEvil" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

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
!MESSAGE "ScatteredEvil - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ScatteredEvil - Win32 Debug" (based on "Win32 (x86) Application")
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
# PROP Output_Dir ".."
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G6 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x426 /d "NDEBUG"
# ADD RSC /l 0x426 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 dinput.lib ddraw.lib dsound.lib dplayx.lib dxguid.lib eaxguid.lib opengl32.lib glu32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "ScatteredEvil - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x426 /d "_DEBUG"
# ADD RSC /l 0x426 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 dinput.lib ddraw.lib dsound.lib dplayx.lib dxguid.lib eaxguid.lib opengl32.lib glu32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"../Debug/ScatteredEvil.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "ScatteredEvil - Win32 Release"
# Name "ScatteredEvil - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\console.cpp
# End Source File
# Begin Source File

SOURCE=.\d_net.cpp
# End Source File
# Begin Source File

SOURCE=.\dd_bind.cpp
# End Source File
# Begin Source File

SOURCE=.\dd_main.cpp
# End Source File
# Begin Source File

SOURCE=.\dd_winit.cpp
# End Source File
# Begin Source File

SOURCE=.\GL_Clip.cpp
# End Source File
# Begin Source File

SOURCE=.\GL_Draw.cpp
# End Source File
# Begin Source File

SOURCE=.\GL_Dyn.cpp
# End Source File
# Begin Source File

SOURCE=.\GL_Font.cpp
# End Source File
# Begin Source File

SOURCE=.\GL_Halo.cpp
# End Source File
# Begin Source File

SOURCE=.\GL_Main.cpp
# End Source File
# Begin Source File

SOURCE=.\GL_MD2.cpp
# End Source File
# Begin Source File

SOURCE=.\GL_Rend.cpp
# End Source File
# Begin Source File

SOURCE=.\GL_RL.cpp
# End Source File
# Begin Source File

SOURCE=.\GL_Sky.cpp
# End Source File
# Begin Source File

SOURCE=.\GL_Tex.cpp
# End Source File
# Begin Source File

SOURCE=.\i_A3D.cpp
# End Source File
# Begin Source File

SOURCE=.\i_DS_EAX.cpp
# End Source File
# Begin Source File

SOURCE=.\i_mixer.cpp
# End Source File
# Begin Source File

SOURCE=.\i_mus.cpp
# End Source File
# Begin Source File

SOURCE=.\i_net.cpp
# End Source File
# Begin Source File

SOURCE=.\i_sound.cpp
# End Source File
# Begin Source File

SOURCE=.\i_timer.cpp
# End Source File
# Begin Source File

SOURCE=.\i_win32.cpp
# End Source File
# Begin Source File

SOURCE=.\jtJoy.cpp
# End Source File
# Begin Source File

SOURCE=.\jtKey.cpp
# End Source File
# Begin Source File

SOURCE=.\jtMouse.cpp
# End Source File
# Begin Source File

SOURCE=.\jtNet.cpp
# End Source File
# Begin Source File

SOURCE=.\p_3dsnd.cpp
# End Source File
# Begin Source File

SOURCE=.\r_data.cpp
# End Source File
# Begin Source File

SOURCE=.\r_draw.cpp
# End Source File
# Begin Source File

SOURCE=.\r_main.cpp
# End Source File
# Begin Source File

SOURCE=.\r_model.cpp
# End Source File
# Begin Source File

SOURCE=.\r_things.cpp
# End Source File
# Begin Source File

SOURCE=.\st_start.cpp
# End Source File
# Begin Source File

SOURCE=.\tga.cpp
# End Source File
# Begin Source File

SOURCE=.\u_args.cpp
# End Source File
# Begin Source File

SOURCE=.\v_video.cpp
# End Source File
# Begin Source File

SOURCE=.\w_wad.cpp
# End Source File
# Begin Source File

SOURCE=.\z_zone.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\anorms.h
# End Source File
# Begin Source File

SOURCE=.\console.h
# End Source File
# Begin Source File

SOURCE=.\dd_api.h
# End Source File
# Begin Source File

SOURCE=.\dd_gl.h
# End Source File
# Begin Source File

SOURCE=.\dd_md2.h
# End Source File
# Begin Source File

SOURCE=.\dd_net.h
# End Source File
# Begin Source File

SOURCE=.\dd_share.h
# End Source File
# Begin Source File

SOURCE=.\GL_Def.h
# End Source File
# Begin Source File

SOURCE=.\GL_Dyn.h
# End Source File
# Begin Source File

SOURCE=.\GL_Font.h
# End Source File
# Begin Source File

SOURCE=.\GL_RL.h
# End Source File
# Begin Source File

SOURCE=.\GL_Sky.h
# End Source File
# Begin Source File

SOURCE=.\GL_Tex.h
# End Source File
# Begin Source File

SOURCE=.\i_A3D.h
# End Source File
# Begin Source File

SOURCE=.\i_console.h
# End Source File
# Begin Source File

SOURCE=.\i_DS_EAX.h
# End Source File
# Begin Source File

SOURCE=.\i_mixer.h
# End Source File
# Begin Source File

SOURCE=.\i_mus.h
# End Source File
# Begin Source File

SOURCE=.\i_net.h
# End Source File
# Begin Source File

SOURCE=.\i_sound.h
# End Source File
# Begin Source File

SOURCE=.\i_timer.h
# End Source File
# Begin Source File

SOURCE=.\i_win32.h
# End Source File
# Begin Source File

SOURCE=.\jtJoy.h
# End Source File
# Begin Source File

SOURCE=.\jtKey.h
# End Source File
# Begin Source File

SOURCE=.\jtMouse.h
# End Source File
# Begin Source File

SOURCE=.\jtNet.h
# End Source File
# Begin Source File

SOURCE=.\jtSound.h
# End Source File
# Begin Source File

SOURCE=.\st_start.h
# End Source File
# Begin Source File

SOURCE=.\tga.h
# End Source File
# Begin Source File

SOURCE=.\u_args.h
# End Source File
# Begin Source File

SOURCE=.\w_wad.h
# End Source File
# Begin Source File

SOURCE=.\z_zone.h
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
# Begin Group "Game Source Files"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=..\ScatteredEvil\a_action.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\am_map.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\con_data.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\con_main.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\con_play.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\con_ui.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\ct_chat.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\f_finale.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\g_game.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\h2_actn.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\h2_main.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\h2_net.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\hconsole.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\hrefresh.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\in_lude.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\info.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\KButtonWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\KCanvas.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\KGC.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\KModalWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\KObject.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\KRootWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\KTextWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\KWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\m_bams.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\m_misc.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mlo_info.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_menu.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_acs.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_anim.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_ceilng.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_doors.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_enemy.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_floor.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_inter.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_lights.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_map.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_maputl.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_mobj.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_npc.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_plats.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_pspr.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_setup.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_sight.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_spec.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_switch.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_telept.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_things.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_tick.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_user.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\po_man.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\s_sound.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\sb_bar.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\sc_man.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\sn_sonix.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\sounds.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\sv_save.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\tables.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\template.cpp
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\x_hair.cpp
# End Source File
# End Group
# Begin Group "Game Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=..\ScatteredEvil\am_data.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\am_map.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\con_local.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\ct_chat.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\h2_actn.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\h2_net.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\h2def.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\i_header.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\info.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\KButtonWindow.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\KCanvas.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\KGC.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\KModalWindow.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\KObject.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\KRootWindow.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\KTextWindow.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\KWindow.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\m_bams.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_base.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_char.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_class.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_controls.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_def.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_effects.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_gameplay.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_gfx.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_info.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_journal.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_joy.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_load.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_main.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_mouse.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_options.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_resolution.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_save.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_skill.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_sound.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\mn_updating.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_local.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\p_spec.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\r_local.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\resource.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\settings.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\sounds.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\soundst.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\textdefs.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\vector.h
# End Source File
# Begin Source File

SOURCE=..\ScatteredEvil\xddefs.h
# End Source File
# End Group
# Begin Group "Config Files"

# PROP Default_Filter "cfg"
# Begin Source File

SOURCE=..\autoexec.cfg
# End Source File
# End Group
# Begin Group "Conversation Files"

# PROP Default_Filter "con"
# Begin Source File

SOURCE=..\Conversations\map01.con
# End Source File
# End Group
# Begin Group "OpenGL driver"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\drOpenGL.h
# End Source File
# Begin Source File

SOURCE=.\glDrv_list.cpp
# End Source File
# Begin Source File

SOURCE=.\glDrv_main.cpp
# End Source File
# Begin Source File

SOURCE=.\glDrv_texture.cpp
# End Source File
# End Group
# Begin Group "Direct3D driver"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\d3d_draw.cpp
# End Source File
# Begin Source File

SOURCE=.\d3d_dxinit.cpp
# End Source File
# Begin Source File

SOURCE=.\d3d_list.cpp
# End Source File
# Begin Source File

SOURCE=.\d3d_main.cpp
# End Source File
# Begin Source File

SOURCE=.\d3d_matrix.cpp
# End Source File
# Begin Source File

SOURCE=.\d3d_misc.cpp
# End Source File
# Begin Source File

SOURCE=.\d3d_state.cpp
# End Source File
# Begin Source File

SOURCE=.\d3d_texture.cpp
# End Source File
# Begin Source File

SOURCE=.\d3d_viewport.cpp
# End Source File
# Begin Source File

SOURCE=.\drD3D.h
# End Source File
# End Group
# End Target
# End Project
