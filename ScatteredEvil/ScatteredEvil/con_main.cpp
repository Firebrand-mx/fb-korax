#include "h2def.h"
#include "p_local.h"
#include "con_local.h"

KConPlay *ConPlay;

//==========================================================================
//
//	CON_StartConversation
//
//==========================================================================

boolean CON_StartConversation(mobj_t *User, mobj_t *Target)
{
	if (Target->type == MT_CMAGE)
	{
		ConPlay = Spawn<KConPlay>();
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
//	CON_Ticker
//
//==========================================================================

void CON_Ticker(void)
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
