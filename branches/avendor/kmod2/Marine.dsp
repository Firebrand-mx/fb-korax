# Microsoft Developer Studio Project File - Name="Marine" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Marine - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Marine.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Marine.mak" CFG="Marine - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Marine - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Marine - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Marine - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "jtnet2" /I "..\utils" /D "NORANGECHECKING" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40b /d "NDEBUG"
# ADD RSC /l 0x40b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 jtnet2.lib ia3dutil.lib dinput.lib dsound.lib eax.lib eaxguid.lib dxguid.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"Release/Doomsday.exe" /libpath:"jtnet2\release"
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "Marine - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "jtnet2" /I "..\utils" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40b /d "_DEBUG"
# ADD RSC /l 0x40b /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 jtnet2.lib ia3dutild.lib dsound.lib eax.lib eaxguid.lib dinput.lib dxguid.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/Doomsday.exe" /pdbtype:sept /libpath:"jtnet2\debug"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "Marine - Win32 Release"
# Name "Marine - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Graphics"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=.\GL_Clip.c
# End Source File
# Begin Source File

SOURCE=.\GL_Draw.c
# End Source File
# Begin Source File

SOURCE=.\GL_Dyn.c
# End Source File
# Begin Source File

SOURCE=.\GL_Font.c
# End Source File
# Begin Source File

SOURCE=.\GL_Halo.c
# End Source File
# Begin Source File

SOURCE=.\GL_Main.c
# End Source File
# Begin Source File

SOURCE=.\GL_MD2.c
# End Source File
# Begin Source File

SOURCE=.\GL_Rend.c
# End Source File
# Begin Source File

SOURCE=.\GL_RL.c
# End Source File
# Begin Source File

SOURCE=.\GL_Sky.c
# End Source File
# Begin Source File

SOURCE=.\GL_Tex.c
# End Source File
# Begin Source File

SOURCE=.\jtrOGL\tga.c
# End Source File
# End Group
# Begin Group "JaKe Tools"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\jtJoy.cpp
# End Source File
# Begin Source File

SOURCE=.\jtKey.cpp
# End Source File
# Begin Source File

SOURCE=.\jtMouse.cpp
# End Source File
# End Group
# Begin Group "Win32 specific"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=.\i_A3D.c
# End Source File
# Begin Source File

SOURCE=.\i_DS_EAX.c
# End Source File
# Begin Source File

SOURCE=.\I_mixer.c
# End Source File
# Begin Source File

SOURCE=.\I_mus.c
# End Source File
# Begin Source File

SOURCE=.\i_sound.c
# End Source File
# Begin Source File

SOURCE=.\i_timer.c
# End Source File
# Begin Source File

SOURCE=.\i_win32.c
# End Source File
# End Group
# Begin Group "Network"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=.\D_net.c
# End Source File
# Begin Source File

SOURCE=.\i_net.c
# End Source File
# End Group
# Begin Group "Refresh"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=.\R_data.c
# End Source File
# Begin Source File

SOURCE=.\R_draw.c
# End Source File
# Begin Source File

SOURCE=.\R_main.c
# End Source File
# Begin Source File

SOURCE=.\R_model.c
# End Source File
# Begin Source File

SOURCE=.\R_plane.c
# End Source File
# Begin Source File

SOURCE=.\R_things.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\Console.c
# End Source File
# Begin Source File

SOURCE=.\dd_3dsnd.c
# End Source File
# Begin Source File

SOURCE=.\dd_actn.c
# End Source File
# Begin Source File

SOURCE=.\dd_bind.c
# End Source File
# Begin Source File

SOURCE=.\dd_main.c
# End Source File
# Begin Source File

SOURCE=.\dd_misc.c
# End Source File
# Begin Source File

SOURCE=.\dd_setup.c
# End Source File
# Begin Source File

SOURCE=.\dd_think.c
# End Source File
# Begin Source File

SOURCE=.\dd_winit.c
# End Source File
# Begin Source File

SOURCE=.\m_bams.c
# End Source File
# Begin Source File

SOURCE=.\St_start.c
# End Source File
# Begin Source File

SOURCE=.\Tables.c
# End Source File
# Begin Source File

SOURCE=.\U_args.c
# End Source File
# Begin Source File

SOURCE=.\V_video.c
# End Source File
# Begin Source File

SOURCE=.\W_wad.c
# End Source File
# Begin Source File

SOURCE=.\Z_zone.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Gfx Headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\dd_gl.h
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

SOURCE=.\gl_s3tc.h
# End Source File
# Begin Source File

SOURCE=.\GL_Sky.h
# End Source File
# Begin Source File

SOURCE=.\GL_Tex.h
# End Source File
# Begin Source File

SOURCE=..\Utils\tga.h
# End Source File
# End Group
# Begin Group "Win32 Headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\i_A3D.h
# End Source File
# Begin Source File

SOURCE=.\i_DS_EAX.h
# End Source File
# Begin Source File

SOURCE=.\I_mixer.h
# End Source File
# Begin Source File

SOURCE=.\I_mus.h
# End Source File
# Begin Source File

SOURCE=.\i_net.h
# End Source File
# Begin Source File

SOURCE=.\I_sound.h
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

SOURCE=.\jtSound.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Console.h
# End Source File
# Begin Source File

SOURCE=.\dd_api.h
# End Source File
# Begin Source File

SOURCE=.\dd_data.h
# End Source File
# Begin Source File

SOURCE=.\dd_def.h
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

SOURCE=.\dd_think.h
# End Source File
# Begin Source File

SOURCE=.\jtNet2\jtNetEx.h
# End Source File
# Begin Source File

SOURCE=.\m_bams.h
# End Source File
# Begin Source File

SOURCE=.\Settings.h
# End Source File
# Begin Source File

SOURCE=.\St_start.h
# End Source File
# Begin Source File

SOURCE=.\U_args.h
# End Source File
# Begin Source File

SOURCE=.\W_wad.h
# End Source File
# Begin Source File

SOURCE=.\Z_zone.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
