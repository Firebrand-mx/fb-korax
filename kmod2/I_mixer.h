#ifndef __DOOMSDAY_MIXER_H__
#define __DOOMSDAY_MIXER_H__

// Mixer return values.
enum
{
	MIX_ERROR = -1,
	MIX_OK
};

// Mixer devices.
enum 
{
	MIX_CDAUDIO
};

// Mixer actions.
enum
{
	MIX_GET,
	MIX_SET
};

// Mixer controls.
enum
{
	MIX_VOLUME			// 0-255
};

int I_InitMixer();
void I_ShutdownMixer();

// Now this is a streamlined API...
int I_Mixer3i(int device, int action, int control);
int I_Mixer4i(int device, int action, int control, int parm);

#endif