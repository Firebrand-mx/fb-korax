#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <objbase.h>
#include "u_args.h"
#include "h2def.h"
#include "i_win32.h"
#include "i_net.h"
#include "i_sound.h"
#include "console.h"
#include "gl_def.h"
#include "resource.h"

// In I_win32.c.
extern void main(int argc, char **argv);

namespace OpenGLDriver { gl_export_t* GetGLAPI(void); }
namespace Direct3DDriver { gl_export_t* GetGLAPI(void); }

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND hWndMain;			// The window handle to the main window.
HINSTANCE hInstApp;		// Instance handle to the application.
cmdargs_t cargs;

// The renderer imports and exports.
gl_export_t		gl;

static int		gfxDriver;


static BOOL CALLBACK ConfigDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, 
							   LPARAM lParam)
{
	HWND it;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		// Init the list boxes.
		it = GetDlgItem(hwndDlg, IDC_GFX_DRIVER_LIST);
		SendMessage(it, LB_ADDSTRING, 0, (LPARAM)"OpenGL");
		SendMessage(it, LB_ADDSTRING, 0, (LPARAM)"Direct3D");
		SendMessage(it, LB_SETCURSEL, (WPARAM)gfxDriver, 0);

		it = GetDlgItem(hwndDlg, IDC_SND_DRIVER_LIST);
		SendMessage(it, LB_ADDSTRING, 0, (LPARAM)"A3D");
		SendMessage(it, LB_ADDSTRING, 0, (LPARAM)"DirectSound / EAX");
		SendMessage(it, LB_SETCURSEL, (WPARAM)use_jtSound, 0);
		
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			// Store the settings.
			gfxDriver = SendMessage(GetDlgItem(hwndDlg, IDC_GFX_DRIVER_LIST),
				LB_GETCURSEL, 0, 0);

			use_jtSound = SendMessage(GetDlgItem(hwndDlg, IDC_SND_DRIVER_LIST),
				LB_GETCURSEL, 0, 0);

			EndDialog(hwndDlg, TRUE);
			return TRUE;

		case IDCANCEL:
			EndDialog(hwndDlg, FALSE);
			return TRUE;
		}
	}
	return FALSE;
}
 
static void InitDrivers(cmdargs_t *cargs)
{
	int i;
	int showconf;

	showconf = GetPrivateProfileInt("Drivers", "CfgNextTime", 1, "./KMod3.ini");
	gfxDriver = GetPrivateProfileInt("Drivers", "GfxDriver", 0, "./KMod3.ini");
	use_jtSound = GetPrivateProfileInt("Drivers", "SndDriver", 1, "./KMod3.ini");

	// See if dialog must be shown.
	for (i = 1; i < cargs->argc; i++)
	{
		if (!stricmp(cargs->argv[i], "-drvdlg"))
		{
			showconf = true;
			break;
		}
	}

	if (showconf || (GetAsyncKeyState(VK_SHIFT) |
		GetAsyncKeyState(VK_CONTROL) | GetAsyncKeyState(VK_MENU)) & 0x8000)
	{
		// Automagically disable config dialog for the next time.
		WritePrivateProfileString("Drivers", "CfgNextTime", "0", "./KMod3.ini");

		if (DialogBox(hInstApp, MAKEINTRESOURCE(IDD_DRVCONFIG), NULL, ConfigDialogProc))
		{
			// Save the settings.
			char buf[80];
			sprintf(buf, "%i", gfxDriver);
			WritePrivateProfileString("Drivers", "GfxDriver", buf, "./KMod3.ini");
			sprintf(buf, "%i", use_jtSound);
			WritePrivateProfileString("Drivers", "SndDriver", buf, "./KMod3.ini");
		}		
	}

	// Do the API transfer.
	if (gfxDriver)
		memcpy(&gl, Direct3DDriver::GetGLAPI(), sizeof(gl));
	else
		memcpy(&gl, OpenGLDriver::GetGLAPI(), sizeof(gl));
}

static BOOL InitApplication(HINSTANCE hInst)
{
	WNDCLASS wc;

	// We need to register a window class for our window.
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = NULL;//LoadIcon(hInst, MAKEINTRESOURCE(IDI_XICON));
	wc.hCursor = NULL; //LoadCursor(hInst, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_ACTIVEBORDER+1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "DoomMainWClass";
	return RegisterClass(&wc);
}

static BOOL InitInstance(HINSTANCE hInst, int cmdShow)
{
	HDC hdc;

	// Create the main window.
	hWndMain = CreateWindow("DoomMainWClass", 
		"Korax "VERSIONTEXT,
		WS_VISIBLE | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInst,
		NULL);
	if(!hWndMain) return FALSE;

	ShowWindow(hWndMain, cmdShow);
	UpdateWindow(hWndMain);

	// Set the font.
	hdc = GetDC(hWndMain);
	SetMapMode(hdc, MM_TEXT);
	SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));
	return TRUE;
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, 
		LPSTR lpCmdLine, int nCmdShow)
{
	FILE *newout;

	// We'll redirect stdout to a log file.
	newout = freopen("KMod3.log", "w", stdout);
	if(!newout)
	{
		MessageBox(NULL, 
			"Redirection of stdout to KMod3.log failed. You won't see anything that's printf()ed.",
			"Damned!", MB_OK | MB_ICONWARNING);
	}

	// Make the instance handle global knowledge.
	hInstApp = hInstance;

	// Prepare the command line arguments.
	PrepareCmdArgs(&cargs, GetCommandLine());

	// Init drivers.
	InitDrivers(&cargs);

	if(!InitApplication(hInstance))
	{
		MessageBox(NULL, "Couldn't initialize application.", "Korax "VERSIONTEXT, 
			MB_OK | MB_ICONERROR);
		return FALSE;
	}
	if(!InitInstance(hInstance, nCmdShow)) 
	{
		MessageBox(NULL, "Couldn't initialize instance.", "Korax "VERSIONTEXT, 
			MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// Initialize the COM.
	CoInitialize(NULL);

	// Fire up the engine. The game loop will also act as the message pump.
	main(cargs.argc, cargs.argv);		

    return 0;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
 
// Shuts down the Doomsday.
void DD_Shutdown()
{
	extern memzone_t *mainzone;

	// Enables Alt-Tab, Alt-Esc, Ctrl-Alt-Del.
	SystemParametersInfo(SPI_SETSCREENSAVERRUNNING, FALSE, 0, 0);

	CoUninitialize();

	S_FreeTextureTypes();
	free(mainzone);
}
 