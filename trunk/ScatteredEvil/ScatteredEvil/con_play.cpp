#include "h2def.h"
//#include "p_local.h"
#include "con_local.h"
#include "vector.h"

IMPLEMENT_CLASS(KConPlay);

static KConChoice TestChoices[2] = {
	{ "Who are you?" },
	{ "What are you doing?" },
};

//==========================================================================
//
//	KConPlay::PlayNextEvent
//
//==========================================================================

bool KConPlay::StartConversation(mobj_t *NewOwner, mobj_t *NewTarget)
{
	if (!NewOwner || !NewOwner->player)
		return false;
	if (NewOwner->health <= 0)
		return false;

	ConOwner = NewOwner;
	ConTarget = NewTarget;

	ConWin = NewWindow(KConWindow, GRootWindow);
	ConWin->SetConPlay(this);

	CurrentSpeaker = ConTarget;
	CurrentSpeakingTo = ConOwner;
	SetCameraMobjs();
	ConWin->DisplayText("How can I help you?");
	con_timer = 3 * TICRATE;

	ConOwner->player->plr->ThirdPersonView = true;
	paused = true;
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

	if (ConWin)
	{
		ConWin->Destroy();
	}

	ConOwner->player->plr->ThirdPersonView = false;

	paused = false;
	bTerminated = true;
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

	if (con_timer)
	{
		con_timer--;
		if (con_timer <= 0)
		{
			PlayNextEvent();
		}
	}
}

//==========================================================================
//
//	KConPlay::SetCameraMobjs
//
//==========================================================================

void KConPlay::SetCameraMobjs(void)
{
	ddplayer_t *plr;
	TVec Pos1;
	TVec Pos2;
	TVec Delta;
	TVec Right;
	TVec CamPos;
	TAVec Angles;

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

	plr = ConOwner->player->plr;
	plr->ThirdPersonOrigin[0] = (fixed_t)(CamPos.x * FRACUNIT);
	plr->ThirdPersonOrigin[1] = (fixed_t)(CamPos.y * FRACUNIT);
	plr->ThirdPersonOrigin[2] = (fixed_t)(CamPos.z * FRACUNIT);
	plr->ThirdPersonAngle = (angle_t)(Angles.yaw * ANG180 / 180.0);
	plr->ThirdPersonPitch = -Angles.pitch;
}

//==========================================================================
//
//	KConPlay::PlayNextEvent
//
//==========================================================================

void KConPlay::PlayNextEvent(void)
{
	if (test_state == 0)
	{
		CurrentSpeaker = ConOwner;
		CurrentSpeakingTo = ConTarget;
		SetCameraMobjs();
		ConWin->Clear();
		ConWin->DisplayChoice(&TestChoices[0]);
		ConWin->DisplayChoice(&TestChoices[1]);
		con_timer = 0;
		test_state = 1;
	}
	else if (test_state == 2)
	{
		CurrentSpeaker = ConTarget;
		CurrentSpeakingTo = ConOwner;
		SetCameraMobjs();
		ConWin->Clear();
		ConWin->DisplayText("I'm an old mage.");
		con_timer = 3 * TICRATE;
		test_state = 4;
	}
	else if (test_state == 3)
	{
		CurrentSpeaker = ConTarget;
		CurrentSpeakingTo = ConOwner;
		SetCameraMobjs();
		ConWin->Clear();
		ConWin->DisplayText("Just walking around.");
		con_timer = 3 * TICRATE;
		test_state = 4;
	}
	else
	{
		TerminateConversation();
	}
}

//==========================================================================
//
//	KConPlay::PlayChoice
//
//==========================================================================

void KConPlay::PlayChoice(KConChoice *Choice)
{
	if (Choice == &TestChoices[0])
		test_state = 2;
	else if (Choice == &TestChoices[1])
		test_state = 3;
	CurrentSpeaker = ConOwner;
	CurrentSpeakingTo = ConTarget;
	SetCameraMobjs();
	ConWin->Clear();
	ConWin->DisplayText(Choice->ChoiceText);
	con_timer = 3 * TICRATE;
}

