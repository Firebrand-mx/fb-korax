#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "u_args.h"
#include "dd_def.h"
#include "i_win32.h"
#include "i_net.h"
#include "i_sound.h"
#include "console.h"
#include "gl_def.h"

// In I_win32.c.
extern void main(int argc, char **argv);

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND hWndMain;			// The window handle to the main window.
HINSTANCE hInstApp;		// Instance handle to the application.
HINSTANCE hInstDGL;		// Instance handle to the rendering DLL.
HINSTANCE hInstGame;	// Instance handle to the game DLL.
cmdargs_t cargs;

// The renderer imports and exports.
gl_import_t		glim;
gl_export_t		gl;

// The game imports and exports.
game_import_t	gi;		
game_export_t	gx;


BOOL InitApplication(HINSTANCE hInst)
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

BOOL InitInstance(HINSTANCE hInst, int cmdShow)
{
	HDC hdc;

	// Create the main window.
	hWndMain = CreateWindow("DoomMainWClass", 
		"Korax "DOOMSDAY_VERSION_TEXT,
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

void *DD_GetDGL(void)
{
	return &gl;
}

void SetGameImports(game_import_t *imp)
{
	memset(imp, 0, sizeof(*imp));

	imp->SetupLevel = DD_SetupLevel;
	imp->PrecacheLevel = R_PrecacheLevel;

	imp->Quit = I_Quit;
	imp->AddStartupWAD = AddWADFile;
	imp->SetConfigFile = DD_SetConfigFile;
	imp->DefineActions = DD_DefineActions;
	imp->Get = DD_GetInteger;
	imp->Set = DD_SetInteger;

	imp->Message = ST_Message;
	imp->Error = I_Error;

	imp->conprintf = CON_Printf;
	imp->flconprintf = CON_FPrintf;
	imp->SetConsoleFont = CON_SetFont;
	imp->AddCommand = CON_AddCommand;
	imp->AddVariable = CON_AddVariable;
	imp->OpenConsole = CON_Open;
	imp->GetCVar = CvarGet;
	imp->Execute = CON_Execute;

	imp->Z_Malloc = Z_Malloc;
	imp->Z_Free = Z_Free;
	imp->Z_FreeTags = Z_FreeTags;
	imp->Z_ChangeTag = Z_ChangeTag2;
	imp->Z_CheckHeap = Z_CheckHeap;

	// Networking.
	imp->GetPlayer = DD_GetPlayer;
	imp->GetPlayerName = I_NetGetPlayerName;
	imp->GetTicCmd = D_GetTicCmd;
	imp->NetGetPlayerData = I_NetGetPlayerData;
	imp->NetGetServerData = I_NetGetServerData;
	imp->NetSetPlayerData = I_NetSetPlayerData;
	imp->NetSetServerData = I_NetSetServerData;
	imp->SendPacket = D_SendCustomPacket;

	// Map.
/*	imp->LoadBlockMap = P_LoadBlockMap;
	imp->LoadReject = P_LoadReject;
	imp->ApproxDistance = P_AproxDistance;
	imp->PointOnLineSide = P_PointOnLineSide;
	imp->BoxOnLineSide = P_BoxOnLineSide;
	imp->MakeDivline = P_MakeDivline;
	imp->PointOnDivlineSide = P_PointOnDivlineSide;
	imp->InterceptVector = P_InterceptVector;
	imp->LineOpening = P_LineOpening;
	imp->LinkThing = P_LinkThing;
	imp->UnlinkThing = P_UnlinkThing;
	imp->BlockLinesIterator = P_BlockLinesIterator;
	imp->BlockThingsIterator = P_BlockThingsIterator;
	imp->PathTraverse = P_PathTraverse;
	imp->CheckSight = P_CheckSight;
*/
	imp->SetSpriteNameList = R_SetSpriteNameList;
	imp->GetSpriteInfo = R_GetSpriteInfo;
	imp->SetBorderGfx = R_SetBorderGfx;
	imp->RenderPlayerView = (void(*)(void*))R_RenderPlayerView;
	imp->ViewWindow = R_SetViewSize;
	imp->R_FlatNumForName = R_FlatNumForName;
	imp->R_CheckTextureNumForName = R_CheckTextureNumForName;
	imp->R_TextureNumForName = R_TextureNumForName;
	imp->R_TextureNameForNum = R_TextureNameForNum;
	imp->R_PointToAngle2 = R_PointToAngle2;
	imp->R_PointInSubsector = (void*(*)(int,int))R_PointInSubsector;

	imp->W_CheckNumForName = W_CheckNumForName;
	imp->W_GetNumForName = W_GetNumForName;
	imp->W_CacheLumpName = W_CacheLumpName;
	imp->W_CacheLumpNum = W_CacheLumpNum;
	imp->W_LumpLength = W_LumpLength;
	imp->W_ReadLump = W_ReadLump;
	imp->W_ChangeCacheTag = W_ChangeCacheTag;

	// Graphics.
	imp->GetDGL = DD_GetDGL;
	imp->Update = DD_GameUpdate;
	imp->ChangeResolution = I_ChangeResolution;
	imp->SetFlatTranslation = R_SetFlatTranslation;
	imp->SetTextureTranslation = R_SetTextureTranslation;
	imp->GL_UseFog = GL_UseWhiteFog;
	imp->ScreenShot = M_ScreenShot;
	imp->GL_ResetData = GL_ResetData;
	imp->GL_ResetTextures = GL_TexReset;
	imp->GL_ClearTextureMem = GL_ClearTextureMemory;
	imp->GL_TexFilterMode = GL_TextureFilterMode;
	imp->GL_SetColorAndAlpha = GL_SetColorAndAlpha;
	imp->GL_SetColor = GL_SetColor;
	imp->GL_SetNoTexture = GL_SetNoTexture;
	imp->GL_SetFlat = GL_SetFlat;
	imp->GL_SetPatch = GL_SetPatch;
	imp->GL_SetRawImage = GL_SetRawImage;
	imp->GL_DrawPatch = GL_DrawPatch;
	imp->GL_DrawFuzzPatch = GL_DrawFuzzPatch;
	imp->GL_DrawAltFuzzPatch = GL_DrawAltFuzzPatch;
	imp->GL_DrawShadowedPatch = GL_DrawShadowedPatch;
	imp->GL_DrawPatchLitAlpha = GL_DrawPatchLitAlpha;
	imp->GL_DrawRawScreen = GL_DrawRawScreen;
	imp->GL_DrawPatch = GL_DrawPatch;
	imp->GL_DrawRect = GL_DrawRect;
	imp->GL_DrawRectTiled = GL_DrawRectTiled;
	imp->GL_DrawCutRectTiled = GL_DrawCutRectTiled;
	imp->GL_DrawPSprite = GL_DrawPSprite;
	imp->GL_SetFilter = GL_SetFilter;
	imp->GL_DrawPatchCS = GL_DrawPatch_CS;
	imp->SkyParams = R_SkyParams;

	imp->GetTime = I_GetTime;
	imp->FrameRate = I_GetFrameRate;
	imp->ClearKeyRepeaters = I_ClearKeyRepeaters;
	imp->EventBuilder = B_EventConverter;
	imp->BindingsForCommand = B_BindingsForCommand;

	imp->InitThinkers = P_InitThinkers;
	imp->AddThinker = P_AddThinker;
	imp->RemoveThinker = P_RemoveThinker;
	imp->RunThinkers = RunThinkers;

	// Sound and music.
	if(use_jtSound)
	{
		// DirectSound(3D) + EAX, if available.
		imp->PlaySound = I_Play2DSound;
		imp->UpdateSound = I_Update2DSound;
		imp->StopSound = I_StopSound;
		imp->SoundIsPlaying = I_SoundIsPlaying;
		imp->Play3DSound = I_Play3DSound;
		imp->Update3DSound = I_Update3DSound;
		imp->UpdateListener = I_UpdateListener;
	}
	else
	{
		// Use A3D 3.0 sound routines.
		imp->PlaySound = I3_PlaySound2D;
		imp->UpdateSound = I3_UpdateSound2D;
		imp->Play3DSound = I3_PlaySound3D;
		imp->Update3DSound = I3_UpdateSound3D;
		imp->UpdateListener = I3_UpdateListener;
		imp->StopSound = I3_StopSound;
		imp->SoundIsPlaying = I3_IsSoundPlaying;
	}
	imp->SetSfxVolume = I_SetSfxVolume;
	
	imp->PlaySong = I_PlaySong;
	imp->SongIsPlaying = I_QrySongPlaying;
	imp->StopSong = I_StopSong;
	imp->PauseSong = I_PauseSong;
	imp->ResumeSong = I_ResumeSong;
	imp->SetMIDIVolume = I_SetMusicVolume;
	imp->SetMusicDevice = I_SetMusicDevice;
	imp->CD = I_CDControl;

	// Misc.
	imp->Argc = Argc;
	imp->Argv = Argv;
	imp->ArgvPtr = ArgvPtr;
	imp->ParmExists = M_ParmExists;
	imp->CheckParm = M_CheckParm;
	imp->ReadFile = M_ReadFile;
	imp->ReadFileClib = M_ReadFileCLib;
	imp->WriteFile = M_WriteFile;
	imp->ExtractFileBase = M_ExtractFileBase;
	imp->ClearBox = M_ClearBox;
	imp->AddToBox = M_AddToBox;

	// Data.
	imp->validcount = &validcount;
/*	imp->topslope = &topslope;
	imp->bottomslope = &bottomslope;*/
	imp->version = DOOMSDAY_VERSION;
	imp->thinkercap = &thinkercap;

	imp->numvertexes = &numvertexes;
	imp->numsegs = &numsegs;
	imp->numsectors = &numsectors;
	imp->numsubsectors = &numsubsectors;
	imp->numnodes = &numnodes;
	imp->numlines = &numlines;
	imp->numsides = &numsides;

	imp->vertexes = (void **)&vertexes;
	imp->segs = (void **)&segs;
	imp->sectors = (void **)&sectors;
	imp->subsectors = (void **)&subsectors;
	imp->nodes = (void **)&nodes;
	imp->lines = (void **)&lines;
	imp->sides = (void **)&sides;

/*	imp->blockmaplump = &blockmaplump;
	imp->blockmap = &blockmap;
	imp->bmapwidth = &bmapwidth;
	imp->bmapheight = &bmapheight;
	imp->bmaporgx = &bmaporgx;
	imp->bmaporgy = &bmaporgy;
	imp->blocklinks = &blocklinks;
	imp->rejectmatrix = &rejectmatrix;*/
}

void SetDGLImports(gl_import_t *imp)
{
	memset(imp, 0, sizeof(*imp));

	imp->apiSize = sizeof(*imp);
	imp->Message = ST_Message;
	imp->Error = I_Error;
	imp->CheckParm = M_CheckParm;
	imp->Argc = Argc;
	imp->Argv = Argv;
}

BOOL InitDGL(cmdargs_t *cargs)
{
	char	*dllName = "drOpenGL.dll";	// The default renderer.
	int		i;
	GETGLAPI GetGLAPI = NULL;

	// See if a specific renderer dll is specified.
	for(i=1; i<cargs->argc; i++)
	{
		if(!stricmp(cargs->argv[i], "-gl") && i<cargs->argc-1)
		{
			dllName = cargs->argv[i+1];
			break;
		}
	}

	// Load the DLL.
	hInstDGL = LoadLibrary(dllName);
	if(!hInstDGL)
	{
		printf( "InitDGL: Loading of %s failed (error %i).\n", dllName, GetLastError());
		return FALSE;
	}
	GetGLAPI = (GETGLAPI) GetProcAddress(hInstDGL, "GetGLAPI");
	if(!GetGLAPI)
	{
		printf( "InitDGL: Failed to get proc address of GetGLAPI (error %d).\n",
			GetLastError());
		return FALSE;
	}
	SetDGLImports(&glim);
	// Do the API transfer.
	memcpy(&gl, GetGLAPI(&glim), sizeof(gl));
	// Old renderers aren't tolerated.
	if(gl.version < DGL_VERSION_NUM)
	{
		printf("InitDGL: Version %d renderer found. Version %d is required.\n", 
			gl.version, DGL_VERSION_NUM);
		return FALSE;
	}
	return TRUE;
}

BOOL InitGameDLL(cmdargs_t *cargs)
{
	char	*dllName = NULL;	// Pointer to the filename of the game DLL.
	int		i;
	GETGAMEAPI GetGameAPI = NULL;

	// First we need to locate the dll name among the command line arguments.
	for(i=1; i<cargs->argc; i++)
	{
		if(!dllName && !stricmp(cargs->argv[i], "-game") && i<cargs->argc-1)
		{
			// This is it.
			dllName = cargs->argv[i+1];			
		}
		if(!stricmp(cargs->argv[i], "-a3d")) use_jtSound = false;
	}
	// Was a game dll specified?
	if(!dllName) 
	{
		dllName="korax.dll";//printf( "InitGameDLL: No game DLL was specified.\n");
//		return FALSE;
	}

	// Now, load the DLL and get the API/exports.
	hInstGame = LoadLibrary(dllName);
	if(!hInstGame)
	{
		printf( "InitGameDLL: Loading of %s failed (error %d).\n", dllName, GetLastError());
		return FALSE;
	}

	// Get the function.
	GetGameAPI = (GETGAMEAPI) GetProcAddress(hInstGame, "GetGameAPI");
	if(!GetGameAPI)
	{
		printf( "InitGameDLL: Failed to get proc address of GetGameAPI (error %d).\n",
			GetLastError());
		return FALSE;
	}		

	// Put the imported stuff into the imports.
	SetGameImports(&gi);

	// Do the API transfer.
	memcpy(&gx, GetGameAPI(&gi), sizeof(gx));

	// Everything seems to be working...
	return TRUE;
}


int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, 
		LPSTR lpCmdLine, int nCmdShow)
{
	FILE *newout;

	// We'll redirect stdout to a log file.
	newout = freopen("Doomsday.out","w",stdout);
	if(!newout)
	{
		MessageBox(NULL, 
			"Redirection of stdout to Doomsday.out failed. You won't see anything that's printf()ed.",
			"Damned!", MB_OK | MB_ICONWARNING);
	}

	// Make the instance handle global knowledge.
	hInstApp = hInstance;

	// Prepare the command line arguments.
	PrepareCmdArgs(&cargs, GetCommandLine());

	// Load the rendering DLL.
	if(!InitDGL(&cargs))
	{
		MessageBox(NULL, "DGL initialization failed. See Doomsday.out for details.",
			"Doomsday "DOOMSDAY_VERSION_TEXT, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// Load the game DLL.
	if(!InitGameDLL(&cargs))
	{
		MessageBox(NULL, "Game DLL initialization failed. See Doomsday.out for details.", 
			"Doomsday "DOOMSDAY_VERSION_TEXT, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	if(!InitApplication(hInstance))
	{
		MessageBox(NULL, "Couldn't initialize application.", "Doomsday "DOOMSDAY_VERSION_TEXT, 
			MB_OK | MB_ICONERROR);
		return FALSE;
	}
	if(!InitInstance(hInstance, nCmdShow)) 
	{
		MessageBox(NULL, "Couldn't initialize instance.", "Doomsday "DOOMSDAY_VERSION_TEXT, 
			MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// Fire up the engine. The game loop will also act as the message pump.
	main(cargs.argc, cargs.argv);		

    return 0;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}
 
// Shuts down the Doomsday.
void DD_Shutdown()
{
	extern memzone_t *mainzone;

	// Enables Alt-Tab, Alt-Esc, Ctrl-Alt-Del.
	SystemParametersInfo(SPI_SETSCREENSAVERRUNNING, FALSE, 0, 0);

	S_FreeTextureTypes();
	free(mainzone);
	FreeLibrary(hInstGame);
	FreeLibrary(hInstDGL);
	hInstGame = NULL;
	hInstDGL = NULL;
}
 