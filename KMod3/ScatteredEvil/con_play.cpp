#include "h2def.h"
#include "p_local.h"
#include "con_local.h"
#include "vector.h"

IMPLEMENT_CLASS(KConPlay);

//==========================================================================
//
//	KConPlay::SetConversation
//
//==========================================================================

void KConPlay::SetConversation(KConversation *NewCon)
{
	Con = NewCon;
}

//==========================================================================
//
//	KConPlay::StartConversation
//
//==========================================================================

bool KConPlay::StartConversation(mobj_t *NewOwner, mobj_t *NewTarget)
{
	//	Make sure we have a valid conversation and a valid Player
	if (!Con || !NewOwner || !NewOwner->player)
		return false;

	//	Make sure the player isn't, uhhrr, you know, DEAD!
	if (NewOwner->health <= 0)
		return false;

	//	Dead actors also ain't talkative.
	if (NewTarget->health <= 0)
		return false;

	//	Keep a pointer to the player and invoking actor
	Player = NewOwner;
	InvokeActor = NewTarget;

	//	Bind the conversation events.
	Con->BindEvents(Player, InvokeActor);

	//	Check to see if all actors are on level.
	//	If a person speaking on the conversation can't be found
	// (say, they were ruthlessly MURDERED!) then abort.
	if (!Con->CheckActors())
		return false;

	CurrentEvent = Con->EventList;

	if (Con->bFirstPerson)
	{
		ConWinFirst = NewWindow(KConWindowFirst, GRootWindow);
		ConWinFirst->SetConPlay(this);
		DisplayMode = DM_FirstPerson;
	}
	else
	{
		ConWinThird = NewWindow(KConWindow, GRootWindow);
		ConWinThird->SetConPlay(this);

		Player->player->ThirdPersonView = true;
		paused = true;
		DisplayMode = DM_ThirdPerson;
	}

	GotoState(CPS_PlayEvent);

	bConversationStarted = true;
	return true;
}

//==========================================================================
//
//	KConPlay::TerminateConversation
//
//==========================================================================

void KConPlay::TerminateConversation(void)
{
	if (bTerminated)
		return;

	if (ConWinFirst)
	{
		ConWinFirst->Destroy();
	}
	if (ConWinThird)
	{
		ConWinThird->Destroy();
	}

	if (bConversationStarted && DisplayMode == DM_ThirdPerson)
	{
		Player->player->ThirdPersonView = false;
		paused = false;
	}

	bTerminated = true;
}

//==========================================================================
//
//	KConPlay::PlayNextEvent
//
//==========================================================================

void KConPlay::PlayNextEvent(void)
{
	ProcessAction(EA_NextEvent, "");
}

//==========================================================================
//
//	KConPlay::PlayChoice
//
//==========================================================================

void KConPlay::PlayChoice(KConChoice *Choice)
{
	//	If this choice has a label, then jump to it. Otherwise just
	// continue with the conversation.
	if (Choice->ChoiceLabel[0])
		ProcessAction(EA_JumpToLabel, Choice->ChoiceLabel);
	else
		ProcessAction(EA_NextEvent, "");
}

//==========================================================================
//
//	KConPlay::ProcessAction
//
//==========================================================================

void KConPlay::ProcessAction(EEventAction NextAction, const char *NextLabel)
{
	//	Don't do squat if the CurrentEvent is NULL
	if (!CurrentEvent)
		return;

	switch (NextAction)
	{
	case EA_NextEvent:
		//	Proceed to the next event.
		LastEvent = CurrentEvent;
		CurrentEvent = CurrentEvent->NextEvent;
		GotoState(CPS_PlayEvent);
		break;

	case EA_JumpToLabel:
		//	Use the label passed back and jump to it
		LastEvent = CurrentEvent;
		CurrentEvent = Con->GetEventFromLabel(NextLabel);
		if (!CurrentEvent)
		{
			CON_Printf("KConPlay::ProcessAction() - EA_JumpToLabel -------\n");
			CON_Printf("  WARNING!  Label %s NOT FOUND in conversation %s\n", NextLabel, Con->ConName);
			CON_Printf("  Conversation terminated\n");
		}
		GotoState(CPS_PlayEvent);
		break;

	case EA_WaitForInput:
		//	We need to wait for some user input before
		// proceeding any further, so jumo into a wait state.
		LastEvent = CurrentEvent;
		GotoState(CPS_WaitForInput);
		break;

	case EA_WaitForText:
		LastEvent = CurrentEvent;
		ConTimer = 3 * TICRATE;
		GotoState(CPS_WaitForText);
		break;

	case EA_End:
		TerminateConversation();
		break;
	}
}

//==========================================================================
//
//	KConPlay::SetupEvent
//
//==========================================================================

void KConPlay::SetupEvent(void)
{
	EEventAction NextEvent = EA_End;
	char NextLabel[32] = "";

	switch (CurrentEvent->EventType)
	{
	case ET_Speech:
		NextEvent = SetupEventSpeech(Cast<KConEventSpeech>(CurrentEvent), NextLabel);
		break;

	case ET_Choice:
		NextEvent = SetupEventChoice(Cast<KConEventChoice>(CurrentEvent), NextLabel);
		break;

	case ET_Jump:
		NextEvent = SetupEventJump(Cast<KConEventJump>(CurrentEvent), NextLabel);
		break;

	case ET_RandomLabel:
		NextEvent = SetupEventRandomLabel(Cast<KConEventRandomLabel>(CurrentEvent), NextLabel);
		break;

	case ET_End:
		NextEvent = SetupEventEnd(Cast<KConEventEnd>(CurrentEvent), NextLabel);
		break;
	}
	ProcessAction(NextEvent, NextLabel);
}

//==========================================================================
//
//	KConPlay::SetupEventSpeech
//
//==========================================================================

EEventAction KConPlay::SetupEventSpeech(KConEventSpeech *Event, char *NextLabel)
{
	if (CurrentSpeaker != Event->Speaker || CurrentSpeakingTo != Event->SpeakingTo)
	{
		CurrentSpeaker = Event->Speaker;
		CurrentSpeakingTo = Event->SpeakingTo;
	}
	SetCameraMobjs();

	CurrentSpeaker->angle = R_PointToAngle2(CurrentSpeaker->x, CurrentSpeaker->y, 
		CurrentSpeakingTo->x, CurrentSpeakingTo->y);
	CurrentSpeakingTo->angle = R_PointToAngle2(CurrentSpeakingTo->x, 
		CurrentSpeakingTo->y, CurrentSpeaker->x, CurrentSpeaker->y);

	//	If we're continuing from the last speech, then we want to Append
	// and not Display the first chunk.
	if (Event->bContinued)
	{
		if (DisplayMode == DM_FirstPerson)
			ConWinFirst->AppendText(Event->ConSpeech->Speech);
		else
			ConWinThird->AppendText(Event->ConSpeech->Speech);
	}
	else
	{
		if (DisplayMode == DM_FirstPerson)
		{
			//	Clear the window.
			ConWinFirst->Clear();

			ConWinFirst->DisplayText(Event->ConSpeech->Speech);
		}
		else
		{
			//	Clear the window.
			ConWinThird->Clear();

			ConWinThird->DisplayText(Event->ConSpeech->Speech);
		}
	}
	return EA_WaitForText;
}

//==========================================================================
//
//	KConPlay::SetupEventChoice
//
//==========================================================================

EEventAction KConPlay::SetupEventChoice(KConEventChoice *Event, char *NextLabel)
{
	KConChoice *Choice;
	int ChoiceCount;

	//	For choices, the speaker is always the player. The person being
	// spoken to, well, that's more complicated (unless I force that to be
	// set in script). For now we'll assume that the owner of the
	// conversation is the person being spoken to.
	CurrentSpeaker = Player;
	CurrentSpeakingTo = InvokeActor;

	//	Update the actors used by the camera
	SetCameraMobjs();

	//	Clear the screen if needed be
	if (Event->bClearScreen)
		ConWinThird->Clear();

	ChoiceCount = 0;
	for (Choice = Event->ChoiceList; Choice; Choice = Choice->NextChoice)
	{
		ConWinThird->DisplayChoice(Choice);
		ChoiceCount++;
	}
	if (ChoiceCount > 0)
		return EA_WaitForInput;
	else
		return EA_NextEvent;
}

//==========================================================================
//
//	KConPlay::SetupEventJump
//
//==========================================================================

EEventAction KConPlay::SetupEventJump(KConEventJump *Event, char *NextLabel)
{
	//	Check to see if the jump label is empty. If so, Then we just want
	// to fall through to the next event. This can happen when jump
	// events get inserted during the inport process. Script will not
	// allow the user to create events like this.
	if (!Event->JumpLabel[0])
	{
		return EA_NextEvent;
	}
	else
	{
		//	Jump to a specific label.
		strcpy(NextLabel, Event->JumpLabel);
		return EA_JumpToLabel;
	}
}

//==========================================================================
//
//	KConPlay::SetupEventRandomLabel
//
//==========================================================================

EEventAction KConPlay::SetupEventRandomLabel(KConEventRandomLabel *Event, char *NextLabel)
{
	strcpy(NextLabel, Event->Labels[P_Random() % Event->NumLabels]);
	return EA_JumpToLabel;
}

//==========================================================================
//
//	KConPlay::SetupEventEnd
//
//==========================================================================

EEventAction KConPlay::SetupEventEnd(KConEventEnd *Event, char *NextLabel)
{
	return EA_End;
}

//==========================================================================
//
//	KConPlay::GotoState
//
//==========================================================================

void KConPlay::GotoState(EConPlayState NewState)
{
	InState = NewState;
}

//==========================================================================
//
//	KConPlay::Tick
//
//==========================================================================

void KConPlay::Tick(void)
{
	if (bTerminated)
		return;

	switch (InState)
	{
	case CPS_PlayEvent:
		if (!CurrentEvent)
			TerminateConversation();
		else
			SetupEvent();
		break;

	case CPS_WaitForInput:
		break;

	case CPS_WaitForText:
		ConTimer--;
		if (ConTimer <= 0)
		{
			PlayNextEvent();
		}
		break;
	}
}

//==========================================================================
//
//	KConPlay::SetCameraMobjs
//
//==========================================================================

void KConPlay::SetCameraMobjs(void)
{
	player_t *plr;
	TVec Pos1;
	TVec Pos2;
	TVec Delta;
	TVec Right;
	TVec CamPos;
	TAVec Angles;

	if (DisplayMode != DM_ThirdPerson)
		return;

	Pos1.x = FIX2FLT(CurrentSpeaker->x);
	Pos1.y = FIX2FLT(CurrentSpeaker->y);
	Pos1.z = FIX2FLT(CurrentSpeaker->z - CurrentSpeaker->floorclip +
		CurrentSpeaker->height / 2);
	Pos2.x = FIX2FLT(CurrentSpeakingTo->x);
	Pos2.y = FIX2FLT(CurrentSpeakingTo->y);
	Pos2.z = FIX2FLT(CurrentSpeakingTo->z - CurrentSpeakingTo->floorclip +
		CurrentSpeakingTo->height / 2);
	Delta = Normalize(Pos1 - Pos2);
	Right = CrossProduct(Delta, TVec(0, 0, 1));
	CamPos = Pos2 - Delta * 48.0 + Right * 48.0 + TVec(0, 0, 16);
	VectorAngles(Pos1 - CamPos, Angles);

	plr = Player->player;
	plr->ThirdPersonOrigin[0] = (fixed_t)(CamPos.x * FRACUNIT);
	plr->ThirdPersonOrigin[1] = (fixed_t)(CamPos.y * FRACUNIT);
	plr->ThirdPersonOrigin[2] = (fixed_t)(CamPos.z * FRACUNIT);
	plr->ThirdPersonAngle = (angle_t)(Angles.yaw * ANG180 / 180.0);
	plr->ThirdPersonPitch = -Angles.pitch;
}

