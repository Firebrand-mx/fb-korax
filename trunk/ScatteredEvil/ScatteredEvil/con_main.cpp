#include "h2def.h"
#include "p_local.h"
#include "con_local.h"

KConPlay *ConPlay;
KConversationList *ConList;

//==========================================================================
//
//	ConSys_LoadConversations
//
//==========================================================================

void ConSys_LoadConversations(char *MapName)
{
	//	Destroy previous conversations, if they still exist
	if (ConList)
	{
		ConList->Destroy();
		ConList = NULL;
	}
	//	And player too
	if (ConPlay)
	{
		ConPlay->Destroy();
		ConPlay = NULL;
	}

	//	Spawn list object
	ConList = Spawn<KConversationList>();

	//	Read script
	ConList->ParseScript(MapName);
}

//==========================================================================
//
//	ConSys_DestroyConversations
//
//==========================================================================

void ConSys_DestroyConversations(void)
{
	if (ConList)
	{
		ConList->Destroy();
		ConList = NULL;
	}
	if (ConPlay)
	{
		ConPlay->Destroy();
		ConPlay = NULL;
	}
}

//==========================================================================
//
//	ConSys_StartConversation
//
//==========================================================================

boolean ConSys_StartConversation(mobj_t *User, mobj_t *Target)
{
	if (!ConList)
		return false;
	if (ConPlay)
	{
		//	Already running a conversation.
		//FIXME Add check wether it can be interrupted.
		return false;
	}
	KConversation *Con = ConList->GetActiveConversation(User, Target);
	if (Con)
	{
		ConPlay = Spawn<KConPlay>();
		ConPlay->SetConversation(Con);
		if (!ConPlay->StartConversation(User, Target))
		{
			ConPlay->Destroy();
			ConPlay = NULL;
			return false;
		}
		return true;
	}
	return false;
}

//==========================================================================
//
//	ConSys_Ticker
//
//==========================================================================

void ConSys_Ticker(void)
{
	if (ConPlay)
	{
		ConPlay->Tick();
		if (ConPlay->Terminated())
		{
			ConPlay->Destroy();
			ConPlay = NULL;
		}
	}
}
