// Common settings/defaults.

#ifndef __DOOMSDAY_SETTINGS_H__
#define __DOOMSDAY_SETTINGS_H__

extern char *defaultWads; // A list of wad names, whitespace in between (in .cfg).
extern int mouseInverseY;
extern int skyDetail, skyRows, mipmapping, linearRaw, texQuality;
extern float skyDist;
extern int useDynLights, borderUpd;
extern int snd_MusicDevice, mlookInverseY;
extern int mouseWheelSensi;
extern int defResX, defResY, simpleSky;
extern int consoleAlpha, consoleLight;
extern int repWait1, repWait2;
extern int mouseDisableX, mouseDisableY;
extern int maxDynLights, dlBlend, clipLights, missileBlend, litSprites;
extern float maxSpriteAngle;
extern int haloMode, flareBoldness, flareSize;
extern int alwaysAlign;
extern int joySensitivity, jlookInverseY, usejoystick;
extern float dlFactor;
extern int filterSprites;
extern int snd_Resample, snd_16bits;
extern float flareFadeMax, flareFadeMin, minFlareSize;
extern int r_ambient;
extern int dlMaxRad;
extern float dlRadFactor;
extern boolean consoleDump;
extern int modelLight, frameInter;
extern float farClip;
extern int l_down,l_up;
extern int time_mod;
extern int act_int;

#endif 