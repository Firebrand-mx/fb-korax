#include "h2def.h"
#include "p_local.h"
#include "con_local.h"

IMPLEMENT_CLASS(KConSpeech);
IMPLEMENT_CLASS(KConChoice);
IMPLEMENT_CLASS(KConEvent);
IMPLEMENT_CLASS(KConversation);
IMPLEMENT_CLASS(KConEventSpeech);
IMPLEMENT_CLASS(KConEventChoice);
IMPLEMENT_CLASS(KConEventJump);
IMPLEMENT_CLASS(KConEventRandomLabel);
IMPLEMENT_CLASS(KConEventEnd);
IMPLEMENT_CLASS(KConversationList);

//==========================================================================
//
//	KConEventSpeech::ParseScript
//
//==========================================================================

void KConEventSpeech::ParseScript(void)
{
	SC_MustGetStringName("{");

	//	Speaker
	SC_MustGetNumber();
	SpeakerTID = sc_Number;

	//	Speaking to
	SC_MustGetNumber();
	SpeakingToTID = sc_Number;

	//	Speech
	SC_MustGetString();
	ConSpeech = Spawn<KConSpeech>();
	strcpy(ConSpeech->Speech, sc_String);

	//	Flags
	while (1)
	{
		SC_MustGetString();
		if (SC_Compare("Continued"))
		{
			bContinued = true;
			SC_MustGetString();
		}
		else
		{
			break;
		}
	}
	SC_UnGet();
	SC_MustGetStringName("}");
}

//==========================================================================
//
//	KConEventChoice::ParseScript
//
//==========================================================================

void KConEventChoice::ParseScript(void)
{
	KConChoice *Prev = NULL;

	SC_MustGetStringName("{");
	bool bDone = false;
	do
	{
		SC_MustGetString();
		if (SC_Compare("Clear") || SC_Compare("ClearScreen"))
		{
			bClearScreen = true;
		}
		else if (SC_Compare("{"))
		{
			KConChoice *Choice = Spawn<KConChoice>();
			//	Text
			SC_MustGetString();
			strcpy(Choice->ChoiceText, sc_String);
			//	Label
			SC_MustGetString();
			strcpy(Choice->ChoiceLabel, sc_String);
			if (!ChoiceList)
			{
				ChoiceList = Choice;
			}
			else
			{
				Prev->NextChoice = Choice;
			}
			Prev = Choice;
			SC_MustGetStringName("}");
		}
		else if (SC_Compare("}"))
		{
			bDone = true;
		}
		else
		{
			SC_ScriptError(NULL);
		}
	} while (!bDone);
}

//==========================================================================
//
//	KConEventJump::ParseScript
//
//==========================================================================

void KConEventJump::ParseScript(void)
{
	SC_MustGetString();
	strcpy(JumpLabel, sc_String);
}

//==========================================================================
//
//	KConEventRandomLabel::ParseScript
//
//==========================================================================

void KConEventRandomLabel::ParseScript(void)
{
	SC_MustGetStringName("{");
	SC_MustGetString();
	while (!SC_Compare("}"))
	{
		if (NumLabels >= 16)
			I_Error("Too many random labels");
		strcpy(Labels[NumLabels], sc_String);
		NumLabels++;
		SC_MustGetString();
	}
	if (!NumLabels)
	{
		SC_ScriptError("No labels specified");
	}
}

//==========================================================================
//
//	KConEventEnd::ParseScript
//
//==========================================================================

void KConEventEnd::ParseScript(void)
{
}

//==========================================================================
//
//	KConversation::ParseScript
//
//==========================================================================

void KConversation::ParseScript(void)
{
	strcpy(ConName, sc_String);

	KConEvent *PrevEvent = NULL;
	SC_MustGetStringName("{");

	//	Flags and params
	while (1)
	{
		SC_MustGetString();
		if (SC_Compare("FirstPerson"))
		{
			bFirstPerson = true;
		}
		else
		{
			SC_UnGet();
			break;
		}
	}

	//	Events
	do
	{
		char NewLabel[32];
		KConEvent *Event = NULL;

		SC_MustGetString();
		//	Check for label
		memset(NewLabel, 0, sizeof(NewLabel));
		if (sc_String[strlen(sc_String) - 1] == ':')
		{
			strcpy(NewLabel, sc_String);
			NewLabel[strlen(NewLabel) - 1] = 0;
			SC_MustGetString();
		}

		if (SC_Compare("speech"))
		{
			Event = Spawn<KConEventSpeech>();
		}
		else if (SC_Compare("choice"))
		{
			Event = Spawn<KConEventChoice>();
		}
		else if (SC_Compare("jump"))
		{
			Event = Spawn<KConEventJump>();
		}
		else if (SC_Compare("RandomLabel"))
		{
			Event = Spawn<KConEventRandomLabel>();
		}
		else if (SC_Compare("end"))
		{
			Event = Spawn<KConEventEnd>();
		}
		else if (SC_Compare("}"))
		{
			break;
		}
		else
		{
			SC_ScriptError(NULL);
		}

		strcpy(Event->Label, NewLabel);
		Event->Conversation = this;
		Event->ParseScript();
		if (!EventList)
		{
			EventList = Event;
		}
		else
		{
			PrevEvent->NextEvent = Event;
		}
		PrevEvent = Event;
	} while (1);
}

//==========================================================================
//
//	KConversation::BindEvents
//
//==========================================================================

void KConversation::BindEvents(mobj_t *pPlayer, mobj_t *InvokeActor)
{
	for (KConEvent *Event = EventList; Event; Event = Event->NextEvent)
	{
		KConEventSpeech *Speech = Cast<KConEventSpeech>(Event);
		if (Speech)
		{
			//	First we check for player, then we check wether TID is equal
			// to InvokeActor's TID, so we can have the same conversation
			// assigned to the multiple acotrs on a level (i.e. generic
			// speeches). Then we search by TID for first living actor.
			if (Speech->SpeakerTID == PLAYER_TID)
			{
				Speech->Speaker = pPlayer;
			}
			else if (Speech->SpeakerTID == InvokeActor->tid)
			{
				Speech->Speaker = InvokeActor;
			}
			else
			{
				int search = -1;
				do
				{
					Speech->Speaker = P_FindMobjFromTID(Speech->SpeakerTID, &search);
				}
				while (Speech->Speaker && Speech->Speaker->health <= 0);
			}

			//	The same for speaking to actor.
			if (Speech->SpeakingToTID == PLAYER_TID)
			{
				Speech->SpeakingTo = pPlayer;
			}
			else if (Speech->SpeakingToTID == InvokeActor->tid)
			{
				Speech->SpeakingTo = InvokeActor;
			}
			else
			{
				int search = -1;
				do
				{
					Speech->SpeakingTo = P_FindMobjFromTID(Speech->SpeakingToTID, &search);
				}
				while (Speech->SpeakingTo && Speech->SpeakingTo->health <= 0);
			}
		}
	}
}

//==========================================================================
//
//	KConversation::CheckActors
//
//==========================================================================

bool KConversation::CheckActors(void)
{
	bool bActorsValid = true;
	for (KConEvent *Event = EventList; Event; Event = Event->NextEvent)
	{
		KConEventSpeech *Speech = Cast<KConEventSpeech>(Event);
		if (Speech)
		{
			if (!Speech->Speaker || !Speech->SpeakingTo)
			{
				bActorsValid = false;
				break;
			}
		}
	}
	return bActorsValid;
}

//==========================================================================
//
//	KConversation::GetEventFromLabel
//
//==========================================================================

KConEvent *KConversation::GetEventFromLabel(const char *Label)
{
	KConEvent *E;

	for (E = EventList; E; E = E->NextEvent)
	{
		if (!stricmp(Label, E->Label))
		{
			return E;
		}
	}
	return NULL;
}

//==========================================================================
//
//	KConversationList::ParseScript
//
//==========================================================================

void KConversationList::ParseScript(const char *MapName)
{
	char FileName[128];

	sprintf(FileName, "Conversations/%s.con", MapName);
	FILE *f = fopen(FileName, "r");
	if (!f)
	{
		//	This map has no conversations. :(
		return;
	}
	fclose(f);
	SC_OpenFile(FileName);

	while (SC_GetString())
	{
		SC_UnGet();
		SC_MustGetNumber();
		int CurrentTID = sc_Number;
		SC_MustGetStringName("{");
		while (1)
		{
			SC_MustGetString();
			if (SC_Compare("}"))
			{
				break;
			}
			KConversation *Con = Spawn<KConversation>();
			Con->ConOwnerTID = CurrentTID;
			Con->ParseScript();
			AddConversation(Con);
		}
	}

	SC_Close();
}

//==========================================================================
//
//	KConversationList::AddConversation
//
//==========================================================================

void KConversationList::AddConversation(KConversation *Con)
{
	if (!Conversations)
	{
		Conversations = Con;
	}
	else
	{
		KConversation *Prev = Conversations;
		while (Prev->NextCon)
		{
			Prev = Prev->NextCon;
		}
		Prev->NextCon = Con;
	}
	Con->NextCon = NULL;
}

//==========================================================================
//
//	KConversationList::GetActiveConversation
//
//==========================================================================

KConversation *KConversationList::GetActiveConversation(mobj_t *pPlayer, mobj_t *InvokeActor)
{
	KConversation *Con;
	bool bAbortConversation;

	bAbortConversation = true;
	for (Con = Conversations; Con; Con = Con->NextCon)
	{
		if (InvokeActor->tid != Con->ConOwnerTID)
			continue;
		bAbortConversation = false;

		if (!bAbortConversation)
			break;
	}
	if (bAbortConversation)
		return NULL;
	else
		return Con;
}

