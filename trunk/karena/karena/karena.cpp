
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>


const int MAX_VALUE_NAME = 16383;
const int MAX_DATA_LENGTH = 32767;
const int COMMAND_LINE_LENGTH = 1024;


TCHAR VavoomCommandLine[COMMAND_LINE_LENGTH];
TCHAR * pVCmdLineCursor;
bool flagDevCustomGame;

enum
{
	REG_VALUE_RENDERER,
	REG_VALUE_RESOLUTION,
	REG_VALUE_PARTICLES,
	REG_VALUE_SCM,
	REG_VALUE_NOSOUND,
	REG_VALUE_NOSFX,
	REG_VALUE_NO3DSOUND,
	REG_VALUE_NOMUSIC,
	REG_VALUE_NOCDAUDIO,
	REG_VALUE_USEOPENAL,
	REG_VALUE_NOLAN,
	REG_VALUE_NOUDP,
	REG_VALUE_NOIPX,
	REG_VALUE_IPADRESS,
	REG_VALUE_PORT,
	REG_VALUE_NOMOUSE,
	REG_VALUE_NOJOY,
	REG_VALUE_DEBUG,
	REG_VALUE_DEVGAME,
	REG_VALUE_FILES,
	REG_VALUE_PROGS,
	REG_VALUE_OPTIONS,
	NUM_REG_VALUES
};

TCHAR * RegValues[NUM_REG_VALUES] = 
{
	L"Renderer",
	L"Resolution",
	L"Particles",
	L"SCM",
	L"NoSound",
	L"NoSfx",
	L"No3DSound",
	L"NoMusic",
	L"NoCDAudio",
	L"UseOpenAL",
	L"NoLAN",
	L"NoUDP",
	L"NoIPX",
	L"IPAddress",
	L"Port",
	L"NoMouse",
	L"NoJoy",
	L"Debug",
	L"DevGame",
	L"Files",
	L"Progs",
	L"Options"
};

void InitVCmdLine()
{
	VavoomCommandLine[0] = '\0';
	pVCmdLineCursor = VavoomCommandLine;
	flagDevCustomGame = false;
}

void ParseRegValue(LPTSTR lpValueName, LPBYTE lpData)
{
	const __w64 int maxchars = (VavoomCommandLine + 1023 - pVCmdLineCursor) >> 1;
	for (int i = 0; i < NUM_REG_VALUES; i++)
	{
		if (wcscmp(lpValueName, RegValues[i]) == 0)
		{
			switch(i)
			{
			case REG_VALUE_RENDERER:
				switch ((UINT)*lpData)
				{
				 case 1:
					swprintf(pVCmdLineCursor, maxchars, L" -opengl");
					pVCmdLineCursor += 8;
					break;
				 case 2:
					swprintf(pVCmdLineCursor, maxchars, L" -d3d");
					pVCmdLineCursor += 4;
					break;
				}
				break;
			case REG_VALUE_RESOLUTION:
				swprintf(pVCmdLineCursor, maxchars, L" +setresolution ");
				pVCmdLineCursor += 16;
				switch ((UINT)*lpData % 6)
				{
				case 0: swprintf(pVCmdLineCursor, maxchars, L"640 480 "); pVCmdLineCursor += 8; break;
				case 1: swprintf(pVCmdLineCursor, maxchars, L"800 600 "); pVCmdLineCursor += 8; break;
				case 2: swprintf(pVCmdLineCursor, maxchars, L"1024 768 "); pVCmdLineCursor += 9; break;
				case 3: swprintf(pVCmdLineCursor, maxchars, L"1152 864 "); pVCmdLineCursor += 9; break;
				case 4: swprintf(pVCmdLineCursor, maxchars, L"1280 1024 "); pVCmdLineCursor += 10; break;
				case 5: swprintf(pVCmdLineCursor, maxchars, L"1600 1200 "); pVCmdLineCursor += 10; break;
				}
				switch ((UINT)*lpData / 6)
				{
				case 0: swprintf(pVCmdLineCursor, maxchars, L"8"); pVCmdLineCursor += 1; break;
				case 1: swprintf(pVCmdLineCursor, maxchars, L"16"); pVCmdLineCursor += 2; break;
				case 2: swprintf(pVCmdLineCursor, maxchars, L"24"); pVCmdLineCursor += 2; break;
				case 3: swprintf(pVCmdLineCursor, maxchars, L"32"); pVCmdLineCursor += 2; break;
				}
				break;
			case REG_VALUE_PARTICLES:
				if (*((TCHAR*)lpData))
				{
					swprintf(pVCmdLineCursor, maxchars, L" -particles %s", (TCHAR*)lpData);
					pVCmdLineCursor += 12 + wcslen((TCHAR*)lpData);
				}
				break;
			case REG_VALUE_SCM:
				if (*((TCHAR*)lpData))
				{
					swprintf(pVCmdLineCursor, maxchars, L" -surfcachesize %s", (TCHAR*)lpData);
					pVCmdLineCursor += 16 + wcslen((TCHAR*)lpData);
				}
				break;
			case REG_VALUE_NOSOUND:
				if (*lpData)
				{
					swprintf(pVCmdLineCursor, maxchars, L" -nosound");
					pVCmdLineCursor += 9;
				}
				break;
			case REG_VALUE_NOSFX:
				if (*lpData)
				{
					swprintf(pVCmdLineCursor, maxchars, L" -nosfx");
					pVCmdLineCursor += 7;
				}
				break;
			case REG_VALUE_NO3DSOUND:
				if (*lpData)
				{
					swprintf(pVCmdLineCursor, maxchars, L" -no3dsound");
					pVCmdLineCursor += 11;
				}
				break;
			case REG_VALUE_NOMUSIC:
				if (*lpData)
				{
					swprintf(pVCmdLineCursor, maxchars, L" -nomusic");
					pVCmdLineCursor += 9;
				}
				break;
			case REG_VALUE_NOCDAUDIO:
				if (*lpData)
				{
					swprintf(pVCmdLineCursor, maxchars, L" -nocdaudio");
					pVCmdLineCursor += 11;
				}
				break;
			case REG_VALUE_USEOPENAL:
				if (*lpData)
				{
					swprintf(pVCmdLineCursor, maxchars, L" -openal");
					pVCmdLineCursor += 8;
				}
				break;
			case REG_VALUE_NOLAN:
				if (*lpData)
				{
					swprintf(pVCmdLineCursor, maxchars, L" -nolan");
					pVCmdLineCursor += 7;
				}
				break;
			case REG_VALUE_NOUDP:
				if (*lpData)
				{
					swprintf(pVCmdLineCursor, maxchars, L" -noudp");
					pVCmdLineCursor += 7;
				}
				break;
			case REG_VALUE_NOIPX:
				if (*lpData)
				{
					swprintf(pVCmdLineCursor, maxchars, L" -noipx");
					pVCmdLineCursor += 7;
				}
				break;
			case REG_VALUE_IPADRESS:
				if (*((TCHAR*)lpData))
				{
					swprintf(pVCmdLineCursor, maxchars, L" -ip %s", (TCHAR*)lpData);
					pVCmdLineCursor += 5 + wcslen((TCHAR*)lpData);
				}
				break;
			case REG_VALUE_PORT:
				if (*((TCHAR*)lpData))
				{
					swprintf(pVCmdLineCursor, maxchars, L" -port %s", (TCHAR*)lpData);
					pVCmdLineCursor += 7 + wcslen((TCHAR*)lpData);
				}
				break;
			case REG_VALUE_NOMOUSE:
				if (*lpData)
				{
					swprintf(pVCmdLineCursor, maxchars, L" -nomouse");
					pVCmdLineCursor += 9;
				}
				break;
			case REG_VALUE_NOJOY:
				if (*lpData)
				{
					swprintf(pVCmdLineCursor, maxchars, L" -nojoy");
					pVCmdLineCursor += 7;
				}
				break;
			case REG_VALUE_DEBUG:
				if (*lpData)
				{
					swprintf(pVCmdLineCursor, maxchars, L" -debug");
					pVCmdLineCursor += 7;
				}
				break;
			case REG_VALUE_DEVGAME:
				flagDevCustomGame = true;
				break;
			case REG_VALUE_FILES:
				if (*((TCHAR*)lpData))
				{
					swprintf(pVCmdLineCursor, maxchars, L" -file %s", (TCHAR*)lpData);
					pVCmdLineCursor += 7 + wcslen((TCHAR*)lpData);
				}
				break;
			case REG_VALUE_PROGS:
				if (*((TCHAR*)lpData))
				{
					swprintf(pVCmdLineCursor, maxchars, L" -progs %s", (TCHAR*)lpData);
					pVCmdLineCursor += 8 + wcslen((TCHAR*)lpData);
				}
				break;
			case REG_VALUE_OPTIONS:
				if (*((TCHAR*)lpData))
				{
					swprintf(pVCmdLineCursor, maxchars, L"%s", (TCHAR*)lpData);
					pVCmdLineCursor += wcslen((TCHAR*)lpData);
				}
				break;
			}
			break;
		}
	}
}

bool ReadSettings()
{
	HKEY hKey;
	if (RegOpenKeyEx( HKEY_CURRENT_USER,
		TEXT("Software\\KoraxTeam\\kalaunch"),
		0,
		KEY_READ,
		&hKey) == ERROR_SUCCESS)
	{
		TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
		DWORD    cchClassName = MAX_PATH;  // size of class string 
		DWORD    cSubKeys=0;               // number of subkeys 
		DWORD    cbMaxSubKey;              // longest subkey size 
		DWORD    cchMaxClass;              // longest class string 
		DWORD    cValues;              // number of values for key 
		DWORD    cchMaxValue;          // longest value name 
		DWORD    cbMaxValueData;       // longest value data 
		DWORD    cbSecurityDescriptor; // size of security descriptor 
		FILETIME ftLastWriteTime;      // last write time 
	 
		DWORD i, retCode; 
	 
		TCHAR achValue[MAX_VALUE_NAME]; 
		DWORD dwValueType;
		BYTE abRealData[MAX_DATA_LENGTH];
	 
		// Get the class name and the value count. 
		retCode = RegQueryInfoKey(
			hKey,                    // key handle 
			achClass,                // buffer for class name 
			&cchClassName,           // size of class string 
			NULL,                    // reserved 
			&cSubKeys,               // number of subkeys 
			&cbMaxSubKey,            // longest subkey size 
			&cchMaxClass,            // longest class string 
			&cValues,                // number of values for this key 
			&cchMaxValue,            // longest value name 
			&cbMaxValueData,         // longest value data 
			&cbSecurityDescriptor,   // security descriptor 
			&ftLastWriteTime);       // last write time 

		if (cValues)
		{
			for (i = 0; i < cValues; i++) 
			{ 
				DWORD cchValue = MAX_VALUE_NAME;
				DWORD cbRealData = MAX_DATA_LENGTH;

				achValue[0] = '\0'; 
				retCode = RegEnumValue(hKey, i, 
					achValue,
					&cchValue,
					NULL,
					&dwValueType,
					abRealData,
					&cbRealData);
	 
				if (retCode == ERROR_SUCCESS ) 
				{ 
					 ParseRegValue(achValue, abRealData);
				}
			}

			return true;
		}
	}
	
	return false;
}

int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nShowCmd)
{
	InitVCmdLine();

	if (!ReadSettings())
	{
		int ret = MessageBox(NULL, L"No custom settings for running Korax Arena were made yet.\nWould you like to open Korax Arena Launcher and configure general options?",
								L"KArena", MB_YESNOCANCEL | MB_ICONINFORMATION);
		if (ret == IDYES)
		{
			if ((__int64)ShellExecute(NULL,
					L"open",
					L"kalaunch.exe",
					NULL,
					NULL,
					SW_SHOWDEFAULT) <= 0x20)
			{
				MessageBox(NULL, L"Failed to run kalaunch.exe", L"KArena", MB_OK | MB_ICONERROR);
			}
			return 0;
		}
		else if (ret == IDCANCEL)
			return 0;
		else
		{
			swprintf(VavoomCommandLine, 1024, L" -game karena");
		}
	}
	else
	{
		swprintf(VavoomCommandLine, 1024, L" -game karena");
	}

	if ((__int64)ShellExecute(NULL,
			L"open",
			L"Vavoom.exe",
			VavoomCommandLine,
			NULL,
			SW_SHOWDEFAULT) <= 0x20)
	{
		MessageBox(NULL, L"Failed to run Vavoom.exe",	L"KArena", MB_OK | MB_ICONERROR);
	}
	
	return 0;
}
