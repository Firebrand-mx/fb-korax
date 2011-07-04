@ECHO OFF
:: Check for:
:: correct Windows version
IF NOT "%OS%"=="Windows_NT"   GOTO Syntax
:: at least 1 command line argument
IF "%~1"=="" GOTO Syntax
:: /? or -? as the first command line argument
ECHO "%~1" | FIND "?" >NUL && GOTO Syntax
:: if the batch file to be tested exists
IF NOT EXIST "%~1" GOTO Syntax

CLS
ECHO.
ECHO User Name       : %UserName%
ECHO Computer Name   : %ComputerName%
ECHO User Domain     : %UserDomain%
FOR /F "tokens=*" %%A IN ('VER') DO ECHO Windows Version : %%A
ECHO Time and Date   : %Date%, %Time%
ECHO.
ECHO Command         : %*

:: Enable delayed variable expansion
SETLOCAL ENABLEDELAYEDEXPANSION

:: In case this batch file is started from
:: an UNC path, map a drive letter first
PUSHD "%~dp1"

:: Remove the line containing the ECHO OFF command
TYPE "%~1" | FIND /I /V "ECHO OFF" > "%~dpn1_Test%~x1"

:: Collect the command line arguments for
:: the batch file that is going to be tested
SET Args=
SET Dummy=
IF NOT "%~2"=="" (
	FOR %%A IN (%*) DO (
		IF "!Dummy!"=="" (
			SET Dummy=%%A
		) ELSE (
			SET Args=!Args! %%A
		)
	)
)

:: Run the "stripped" temporary copy of the batch
:: file with its arguments and log every command
ECHO ON
@CALL "%~dpn1_Test%~x1"%Args% > "%~dpn1_Test.log" 2>&1
@SET ReturnCode=%ErrorLevel%
@ECHO OFF

:: Remove the "stripped" temporary batch file
DEL "%~dpn1_Test%~x1"

:: Display return code and log file name
ECHO.
ECHO Return Code     : %ReturnCode% ^(maybe^)
ECHO Log File        : %~dpn1_Test%~x1

:: Remove the drive mapping
POPD

:: Purge the local settings
ENDLOCAL

:: Wait 1 minute if the batch file was started by doubleclicking
IF /I "%~0"=="%~f0" PING 127.0.0.1 -n 60 >NUL 2>&1

:: Done
GOTO:EOF


:Syntax
ECHO LogBatch.bat,  Version 1.00 for Windows 2000 and later
ECHO Run a batch file and log each individual command line and its result
ECHO.
ECHO Usage:  LOGBATCH.BAT  some_bat.bat  [ optional arguments for some_bat ]
ECHO.
ECHO Where:  "some_bat.bat" is the batch file to be logged
ECHO.
ECHO Note:   "some_bat.bat" may have either a .bat or .cmd extension,
ECHO         but it MUST be a batch file, not an executable.
ECHO.
ECHO Issues: LogBatch.bat MAY fail on switches like ^/? or ^/A in
ECHO         some_bat.bat's optional command line arguments.
ECHO         The displayed value for the return code is not reliable.
ECHO.
ECHO Written by Rob van der Woude
ECHO http://www.robvanderwoude.com
