// Common settings/defaults.

#ifndef __JHEXEN_SETTINGS_H__
#define __JHEXEN_SETTINGS_H__

extern int screenblocks;
extern char *SavePath;

extern int chooseAndUse, lookdirSpeed;
extern boolean fastMonsters;

extern int skyDetail, mipmapping, linearRaw;
extern int useDynLights, showFPS, lookSpring;
extern int snd_MusicDevice, mlookInverseY;
extern int echoMsg, simpleSky;
extern int translucentIceCorpse, consoleAlpha, consoleLight;
extern int repWait1, repWait2;
extern int maxDynLights, dlBlend, missileBlend;
extern int haloMode, flareBoldness, flareSize;

extern byte netMobDamageModifier; // multiplier for non-player mobj damage
extern byte netMobHealthModifier; // health modifier for non-player mobjs
extern byte overrideHubMsg; // skip the transition hub message when 1
extern byte demoDisabled; // disable demos

extern int usemouse, noAutoAim;
extern boolean povLookAround;
extern int joySensitivity, jlookInverseY;

extern int xhair, xhairSize;
extern byte xhairColor[3];

extern int /*snd_Channels, */snd_3D, messageson;
extern char *chat_macros[9];
extern float snd_ReverbFactor;
extern boolean reverbDebug;

extern int dclickuse;

#endif //__JHEXEN_SETTINGS_H__
