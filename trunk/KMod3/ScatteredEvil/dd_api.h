#ifndef __DOOMSDAY_GAME_API_H__
#define __DOOMSDAY_GAME_API_H__

#include "dd_share.h"
#include "dd_gl.h"

void ST_Message(char *msg, ...);
void I_Error(char *error, ...);
void DD_CoreDump(const char *fmt, ...);

// Refresh.
void R_SetSpriteNameList(char **namelist);
void R_GetSpriteInfo(int sprite, int frame, spriteinfo_t *sprinfo);
void R_SetBorderGfx(char *lumps[9]);
void R_RenderPlayerView(struct player_t *player);
void R_SetViewSize(int x, int y, int w, int h);
int R_FlatNumForName(char *name);
int R_CheckTextureNumForName(char *name);
int R_TextureNumForName(char *name);
char* R_TextureNameForNum(int num);
int I_GetFrameRate(void);

	// Graphics.
void DD_GameUpdate(int flags);
int I_ChangeResolution(int width, int height);
int R_SetFlatTranslation(int flat, int translateTo);
int R_SetTextureTranslation(int tex, int translateTo);
void GL_UseWhiteFog(int yes);
int M_ScreenShot(char *filename, int bits);
void GL_ResetData(void);
void GL_TexReset(void);
void GL_ClearTextureMemory(void);
void GL_TextureFilterMode(int target, int parm);
void GL_SetColorAndAlpha(float r, float g, float b, float a);
void GL_SetColor(int palidx);	// Uses playpal
void GL_SetNoTexture();
void GL_SetFlat(int idx);
void GL_SetPatch(int lump);
void GL_SetRawImage(int lump, int part);
void GL_DrawPatch(int x, int y, int lumpnum);
void GL_DrawFuzzPatch(int x, int y, int lumpnum);
void GL_DrawAltFuzzPatch(int x, int y, int lumpnum);
void GL_DrawShadowedPatch(int x, int y, int lumpnum);
void GL_DrawPatchLitAlpha(int x, int y, float light, float alpha, int lumpnum);
void GL_DrawPatch_CS(int x, int y, int lumpnum);
void GL_DrawRawScreen(int lump);
void GL_DrawRect(float x, float y, float w, float h, float r, 
					float g, float b, float a);
void GL_DrawRectTiled(int x, int y, int w, int h, int tw, int th);
void GL_DrawCutRectTiled(int x, int y, int w, int h, int tw, 
					int th, int cx, int cy, int cw, int ch);
void GL_DrawPSprite(int x, int y, float scale, int flip, int lump);
void GL_SetFilter(int rgba);
void R_SkyParams(int layer, int parm, float value);

// Sound.
int SND_PlaySound(void *data, int volume, int pan, int pitch);
void SND_UpdateSound(int handle, int volume, int pan, int pitch);
int SND_Play3DSound(void *data, sound3d_t *desc);
void SND_Update3DSound(int handle, sound3d_t *desc);
void SND_UpdateListener(listener3d_t *desc);
void SND_StopSound(int handle);
int SND_SoundIsPlaying(int handle);
void I_SetSfxVolume(int volume);	

// MIDI music.
int I_PlaySong(void *data, int length, int loop);
int I_QrySongPlaying(void);
void I_StopSong(void);
void I_PauseSong(void);
void I_ResumeSong(void);
void I_SetMusicDevice(int musicdevice);
void I_SetMusicVolume(int volume);

// CD-ROM.	
int I_CDControl(int cmd, int parm);

// Console.
void CON_Printf(char *format, ...);
void CON_FPrintf(int flags, char *format, ...);
void CON_AddCommand(ccmd_t *command);
void CON_AddVariable(cvar_t *variable);	
void CON_Open(int open);
cvar_t* CvarGet(char *name);
int CON_Execute(char *command, int silent);

// Networking.
char* I_NetGetPlayerName(int playerNum);
void D_GetTicCmd(void *cmd, int player);
int I_NetSetServerData(void *data, int size);
int I_NetGetServerData(void *data, int size);
int I_NetSetPlayerData(void *data, int size);
int I_NetGetPlayerData(int playerNum, void *data, int size);

void R_PrecacheLevel(void);

// Low-level stuff.
int I_GetTime(void);	
void I_ClearKeyRepeaters(void);
void B_EventConverter(char *buff, event_t *ev, boolean to_event);
int B_BindingsForCommand(char *command, char *buffer);

// Base-level.
void AddWADFile(char *file);
void I_Quit(void);

//
// DATA
//
extern gl_export_t		gl;

// The routines/data exported from the game DLL.

// Base-level.
void H2_PreInit(void);
void H2_PostInit(void);
void H2_Shutdown(void);
void H2_UpdateState(int step);

// Ticcmds.
void G_BuildTiccmd(struct ticcmd_t *cmd);

// Networking.
int H2_NetServerOpen(int before);
int H2_NetServerStarted(int before);
int H2_NetServerClose(int before);
int H2_NetConnect(int before);
int H2_NetDisconnect(int before);
int H2_NetPlayerEvent(int playernum, int type, void *data);

// Tickers.
void G_Ticker(void);
void H2_Ticker(void);

// Responders.
boolean H2_PrivilegedResponder(event_t *event);
boolean MN_Responder(event_t *event);
boolean G_Responder(event_t *event);

// Refresh.
void H2_EndFrame(void);
void G_Drawer(void);
void MN_Drawer(void);
void R_DrawPlayerSprites(void);
void H2_ConsoleBg(int *width, int *height);

// Miscellaneous.
void P_MobjThinker(mobj_t *mo);

#endif