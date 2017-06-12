@ECHO OFF
REM This batch file receives 5 parameters, a filename, a file extension, the path of the file stripped from drive letter
REM and a full directory name, it appends these parameters in a special sequence to generate a .ls file for a single directory
REM directories are handled by pk3s.bat and directory.bat (if you want to know more open those files...)

REM Now, depending on the type of files we are using, we need to use different directories like this...
if %5 == data goto :Data
if %5 == graphics goto :Graphics
if %5 == maps goto :Maps
if %5 == models goto :Models
if %5 == music goto :Music
if %5 == skyboxespack goto :Skyboxes
if %5 == sounds goto :Sounds
if %5 == sprites goto :Sprites
if %5 == texpak goto :TexPak
if %5 == texpak2 goto :TexPak2
if %5 == texpak3 goto :TexPak3
if %5 == textures goto :Textures

:Data
REM Data files go in the root of the PK3 file
echo %1%2          %4\%1%2>>%CD%\%5.ls
goto :End

:Graphics
REM Graphics go inside the 'graphics' directory
echo graphics\%1%2          %4\%1%2>>%CD%\%5.ls
goto :End

:Maps
REM Maps go in the root of the PK3 file
echo %1%2          %4\%1%2>>%CD%\%5.ls
goto :End

:Models
REM echo models\%1%2          %4\%1%2>>%CD%\%5.ls
goto :End

:Music
echo music\%1%2          %4\%1%2>>%CD%\%5.ls
goto :End

:Skyboxes
REM echo skies\%1%2          %4\%1%2>>%CD%\%5.ls
goto :End

:Sounds
REM echo sounds\%1%2          %4\%1%2>>%CD%\%5.ls
goto :End

:Sprites
echo sprites\%1%2          %4\%1%2>>%CD%\%5.ls
goto :End

:TexPak
REM echo textures\%1%2          %4\%1%2>>%CD%\%5.ls
goto :End

:TexPak2
REM echo textures\%1%2          %4\%1%2>>%CD%\%5.ls
goto :End

:TexPak3
REM echo textures\%1%2          %4\%1%2>>%CD%\%5.ls
goto :End

:Textures
echo %1%2          %4\%1%2>>%CD%\%5.ls
goto :End

:End
