# Microsoft Developer Studio Project File - Name="DoomsDay" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=DoomsDay - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DoomsDay.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DoomsDay.mak" CFG="DoomsDay - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DoomsDay - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "DoomsDay - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DoomsDay - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../jtNet2" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "NORANGECHECKING" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x426 /d "NDEBUG"
# ADD RSC /l 0x426 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ia3dutil.lib dinput.lib dsound.lib eax.lib eaxguid.lib dxguid.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "DoomsDay - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../jtNet2" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x426 /d "_DEBUG"
# ADD RSC /l 0x426 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ia3dutil.lib dinput.lib dsound.lib eax.lib eaxguid.lib dxguid.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "DoomsDay - Win32 Release"
# Name "DoomsDay - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Console.cpp
# End Source File
# Begin Source File

SOURCE=.\d_net.cpp
# End Source File
# Begin Source File

SOURCE=.\dd_3dsnd.cpp
# End Source File
# Begin Source File

SOURCE=.\dd_actn.cpp
# End Source File
# Begin Source File

SOURCE=.\dd_bind.cpp
# End Source File
# Begin Source File

SOURCE=.\dd_main.cpp
# End Source File
# Begin Source File

SOURCE=.\dd_misc.cpp
# End Source File
# Begin Source File

SOURCE=.\dd_setup.cpp
# End Source File
# Begin Source File

SOURCE=.\dd_think.cpp
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

SOURCE=.\I_mixer.cpp
# End Source File
# Begin Source File

SOURCE=.\I_mus.cpp
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

SOURCE=.\m_bams.cpp
# End Source File
# Begin Source File

SOURCE=.\R_data.cpp
# End Source File
# Begin Source File

SOURCE=.\R_draw.cpp
# End Source File
# Begin Source File

SOURCE=.\R_main.cpp
# End Source File
# Begin Source File

SOURCE=.\R_model.cpp
# End Source File
# Begin Source File

SOURCE=.\R_things.cpp
# End Source File
# Begin Source File

SOURCE=.\St_start.cpp
# End Source File
# Begin Source File

SOURCE=.\Tables.cpp
# End Source File
# Begin Source File

SOURCE=.\Template.cpp
# End Source File
# Begin Source File

SOURCE=.\tga.cpp
# End Source File
# Begin Source File

SOURCE=.\U_args.cpp
# End Source File
# Begin Source File

SOURCE=.\V_video.cpp
# End Source File
# Begin Source File

SOURCE=.\W_wad.cpp
# End Source File
# Begin Source File

SOURCE=.\Z_zone.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\anorms.h
# End Source File
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

SOURCE=.\dd_think.h
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

SOURCE=.\GL_s3tc.h
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

SOURCE=.\tga.h
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
