
//**************************************************************************
//**
//** I_MIXER.C
//**
//**************************************************************************

// HEADER FILES ------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include "dd_def.h"
#include "st_start.h"
#include "i_mixer.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern int sounddebug;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

static int					initOk = 0;
static MMRESULT				res;
static HMIXER				mixer = NULL;
static MIXERLINE			mlineCD;
static MIXERLINECONTROLS	mlineCDControls;
static MIXERCONTROL			mctrlCDVolume;

// CODE --------------------------------------------------------------------

// A ridiculous amount of code to do something this simple.
// But mixers are a bit abstract subject, I guess...
int I_InitMixer()
{
	MIXERCAPS	mixerCaps;
	int			num = mixerGetNumDevs();	// Number of mixer devices.

	if(initOk || M_CheckParm("-nomixer")) return 1;

	if(sounddebug) ST_Message( "mixer devices: %d\n", num);	

	// Open the mixer device.
	res = mixerOpen(&mixer, 0, 0, 0, MIXER_OBJECTF_MIXER);
	if(res != MMSYSERR_NOERROR)
	{
		if(sounddebug) ST_Message( "error opening mixer: error %d\n", res);
		return 0;
	}
	
	// Get the device caps.
	mixerGetDevCaps( (UINT) mixer, &mixerCaps, sizeof(mixerCaps));

	ST_Message( "I_InitMixer: %s\n", mixerCaps.szPname);
	
	if(sounddebug) ST_Message( "Audio line destinations: %d\n", mixerCaps.cDestinations);

	memset(&mlineCD, 0, sizeof(mlineCD));
	mlineCD.cbStruct = sizeof(mlineCD);
	mlineCD.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC;

	res = mixerGetLineInfo( (HMIXEROBJ) mixer, &mlineCD, MIXER_GETLINEINFOF_COMPONENTTYPE);
	if(res != MMSYSERR_NOERROR)
	{
		if(sounddebug) ST_Message( "error getting CD audio line info: error %d\n", res);
		return 0;
	}

	if(sounddebug)
	{
		ST_Message( "Dest. line idx: %d\n", mlineCD.dwDestination);
		ST_Message( "Line ID: %x\n", mlineCD.dwLineID);
		ST_Message( "Channels: %d\n", mlineCD.cChannels);
		ST_Message( "Controls: %d\n", mlineCD.cControls);
		ST_Message( "Name: %s (%s)\n", mlineCD.szName, mlineCD.szShortName);
	}

	memset(&mlineCDControls, 0, sizeof(mlineCDControls));
	mlineCDControls.cbStruct = sizeof(mlineCDControls);
	mlineCDControls.dwLineID = mlineCD.dwLineID;
	mlineCDControls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
	mlineCDControls.cControls = 1;
	mlineCDControls.cbmxctrl = sizeof(mctrlCDVolume);
	mlineCDControls.pamxctrl = &mctrlCDVolume;

	res = mixerGetLineControls( (HMIXEROBJ) mixer, &mlineCDControls, 
		MIXER_GETLINECONTROLSF_ONEBYTYPE);
	if(res != MMSYSERR_NOERROR)
	{
		if(sounddebug) ST_Message( "error getting CD audio line controls (vol): error %d\n", res);
		return 0;
	}

	if(sounddebug)
	{
		ST_Message( "Volume control ID: %x\n", mctrlCDVolume.dwControlID);
		ST_Message( "Name: %s (%s)\n", mctrlCDVolume.szName, mctrlCDVolume.szShortName);
		ST_Message( "Min: %d\nMax: %d\n", mctrlCDVolume.Bounds.dwMinimum, 
			mctrlCDVolume.Bounds.dwMaximum);
	}

	// We're successful.
	initOk = 1;
	return 1;
}

void I_ShutdownMixer()
{
	if(!initOk) return; // Can't uninitialize if not inited.

	mixerClose(mixer);
	mixer = NULL;
	initOk = 0;
}

int I_Mixer4i(int device, int action, int control, int parm)
{
	MIXERCONTROLDETAILS				ctrlDetails;
	MIXERCONTROLDETAILS_UNSIGNED	mcdUnsigned[2];
	MIXERCONTROL					*mctrl;
	MIXERLINE						*mline;
	int								i;

	if(!initOk) return MIX_ERROR;

	// This is quite specific at the moment.
	if(device != MIX_CDAUDIO) return MIX_ERROR;
	if(control != MIX_VOLUME) return MIX_ERROR;

	mline = &mlineCD;
	mctrl = &mctrlCDVolume;

	// Init the data structure.
	memset(&ctrlDetails, 0, sizeof(ctrlDetails));
	ctrlDetails.cbStruct = sizeof(ctrlDetails);
	ctrlDetails.dwControlID = mctrl->dwControlID;
	ctrlDetails.cChannels = mline->cChannels;
	ctrlDetails.cbDetails = sizeof(mcdUnsigned);
	ctrlDetails.paDetails = &mcdUnsigned;

	switch(action)
	{
	case MIX_GET:
		res = mixerGetControlDetails( (HMIXEROBJ) mixer, &ctrlDetails, 
			MIXER_GETCONTROLDETAILSF_VALUE);
		if(res != MMSYSERR_NOERROR) return MIX_ERROR;
		
		// The bigger one is the real volume.
		i = mcdUnsigned[mcdUnsigned[0].dwValue > mcdUnsigned[1].dwValue? 0 : 1].dwValue;
		
		// Return the value in range 0-255.
		return (255*(i - mctrl->Bounds.dwMinimum)) / (mctrl->Bounds.dwMaximum - mctrl->Bounds.dwMinimum);

	case MIX_SET:
		// Clamp it.
		if(parm < 0) parm = 0;
		if(parm > 255) parm = 255;

		// Set both channels to the same volume (center balance).
		mcdUnsigned[0].dwValue = mcdUnsigned[1].dwValue = 
			(parm*(mctrl->Bounds.dwMaximum - mctrl->Bounds.dwMinimum))/255 + mctrl->Bounds.dwMinimum;

		/*mcdUnsigned.dwValue = parm + (parm<<8);*/
//		mcdUnsigned.dwValue = ;
		//mcdUnsigned.dwValue += mcdUnsigned.dwValue << 16;
		//ST_Message("set cdvol: %x\n", mcdUnsigned.dwValue);
		res = mixerSetControlDetails( (HMIXEROBJ) mixer, &ctrlDetails,
			MIXER_SETCONTROLDETAILSF_VALUE);
		if(res != MMSYSERR_NOERROR) return MIX_ERROR;		
		break;

	default:
		return MIX_ERROR;
	}
	return MIX_OK;
}

int I_Mixer3i(int device, int action, int control)
{
	return I_Mixer4i(device, action, control, 0);	
}

//==========================================================================
//
//
//
//==========================================================================
