#include "mn_def.h"

//==========================================================================
//
//	Conversation objects
//
//==========================================================================

#define PLAYER_TID	-1

enum EEventAction
{
	EA_NextEvent,
	EA_JumpToLabel,
	//EA_JumpToConversation,
	EA_WaitForInput,
	EA_WaitForSpeech,
	EA_WaitForText,
	EA_PlayAnim,
	EA_ConTurnActors,
	EA_End
};

enum EConEventType
{
	ET_Speech,
	ET_Choice,
	ET_Jump,
	ET_RandomLabel,
	ET_End
};

enum EConPlayState
{
	CPS_PlayEvent,
	CPS_WaitForInput,
	CPS_WaitForText,
};

class KConversation;

//
//	KConSpeech
//
class KConSpeech:public KObject
{
	DECLARE_CLASS(KConSpeech, KObject, 0);
	NO_DEFAULT_CONSTRUCTOR(KConSpeech);

	char Speech[256];
};

//
//	KConChoice
//
class KConChoice:public KObject
{
	DECLARE_CLASS(KConChoice, KObject, 0);
	NO_DEFAULT_CONSTRUCTOR(KConChoice);

	char ChoiceText[256];		// Choice text
	char ChoiceLabel[32];		// Choice label

	bool bDisplayAsSpeech;		// Display choice as speech after user selects it

	KConChoice *NextChoice;		// Pointer to next choice
};

//
//	KConEvent
//
class KConEvent:public KObject
{
	DECLARE_ABSTRACT_CLASS(KConEvent, KObject, 0);
	NO_DEFAULT_CONSTRUCTOR(KConEvent);

	EConEventType EventType;	// Event type
	char Label[32];				// Optional event label
	KConEvent *NextEvent;		// Pointer to next event
	KConversation *Conversation;// Conversation that owns this event

	virtual void ParseScript(void) = 0;
};

//
//	KConversation
//
class KConversation:public KObject
{
	DECLARE_CLASS(KConversation, KObject, 0);
	NO_DEFAULT_CONSTRUCTOR(KConversation);

	char ConName[32];			// Conversation name
	int ConOwnerTID;			// Conversation owner TID
	bool bFirstPerson;			// Remain in First-Person mode
	//bool bRandomCamera;		// Random camera placement (can be overriden)

	KConEvent *EventList;		// First event

	int ConID;					// Internal conversation ID

	KConversation *NextCon;		// Pointer to next conversation

	void ParseScript(void);
	void BindEvents(mobj_t *pPlayer, mobj_t *InvokeActor);
	bool CheckActors(void);
	KConEvent *GetEventFromLabel(const char *Label);
};

//
//	KConEventSpeech
//
class KConEventSpeech:public KConEvent
{
	DECLARE_CLASS(KConEventSpeech, KConEvent, 0);
	
	mobj_t *Speaker;			// Mobj who speaks
	int SpeakerTID;				// Mobj TID
	mobj_t *SpeakingTo;			// Mobj who is being spoken to
	int SpeakingToTID;			// Mobj TID
	KConSpeech *ConSpeech;		// Speech
	bool bContinued;			// True if this speech continued from last speech event

	KConEventSpeech(void)
	{
		EventType = ET_Speech;
	}
	void ParseScript(void);
};

//
//	KConEventChoice
//
class KConEventChoice:public KConEvent
{
	DECLARE_CLASS(KConEventChoice, KConEvent, 0);

	bool bClearScreen;
	KConChoice *ChoiceList;

	KConEventChoice(void)
	{
		EventType = ET_Choice;
	}
	void ParseScript(void);
};

//
//	KConEventJump
//
class KConEventJump:public KConEvent
{
	DECLARE_CLASS(KConEventJump, KConEvent, 0);

	char JumpLabel[32];

	KConEventJump(void)
	{
		EventType = ET_Jump;
	}
	void ParseScript(void);
};

//
//	KConEventRandomLabel
//
class KConEventRandomLabel:public KConEvent
{
	DECLARE_CLASS(KConEventRandomLabel, KConEvent, 0);

	int NumLabels;
	char Labels[16][32];

	KConEventRandomLabel(void)
	{
		EventType = ET_RandomLabel;
	}
	void ParseScript(void);
};

//
//	KConEventEnd
//
class KConEventEnd:public KConEvent
{
	DECLARE_CLASS(KConEventEnd, KConEvent, 0);

	KConEventEnd(void)
	{
		EventType = ET_End;
	}
	void ParseScript(void);
};

//
//	KConversationList
//
//	Maintains a collection of Conversation objects
//
class KConversationList:public KObject
{
	DECLARE_CLASS(KConversationList, KObject, 0);
	NO_DEFAULT_CONSTRUCTOR(KConversationList);

	KConversation *Conversations;

	void ParseScript(const char *MapName);
	void AddConversation(KConversation *Con);
	KConversation *GetActiveConversation(mobj_t *pPlayer, mobj_t *InvokeActor);
};

//==========================================================================
//
//	Playback of conversations
//
//==========================================================================

class KConPlay:public KObject
{
	DECLARE_CLASS(KConPlay, KObject, 0);
	NO_DEFAULT_CONSTRUCTOR(KConPlay);

protected:
	enum EDisplayMode
	{
		DM_FirstPerson,
		DM_ThirdPerson
	} DisplayMode;

	EConPlayState InState;
	KConversation *Con;
	KConEvent *CurrentEvent;
	KConEvent *LastEvent;
	bool bConversationStarted;
	bool bTerminated;

	mobj_t *Player;
	mobj_t *InvokeActor;

	class KConWindow *ConWinThird;
	class KConWindowFirst *ConWinFirst;

	mobj_t *CurrentSpeaker;
	mobj_t *CurrentSpeakingTo;

	int ConTimer;

public:
	void SetConversation(KConversation *NewCon);
	bool StartConversation(mobj_t *NewOwner, mobj_t *NewTarget);
	void TerminateConversation(void);

	void PlayNextEvent(void);
	void PlayChoice(KConChoice *Choice);
	void ProcessAction(EEventAction NextAction, const char *NextLabel);

	void SetupEvent(void);
	EEventAction SetupEventSpeech(KConEventSpeech *Event, char *NextLabel);
	EEventAction SetupEventChoice(KConEventChoice *Event, char *NextLabel);
	EEventAction SetupEventJump(KConEventJump *Event, char *NextLabel);
	EEventAction SetupEventRandomLabel(KConEventRandomLabel *Event, char *NextLabel);
	EEventAction SetupEventEnd(KConEventEnd *Event, char *NextLabel);

	void GotoState(EConPlayState NewState);
	void Tick(void);

	void SetCameraMobjs(void);

	bool ConversationStarted(void) { return bConversationStarted; }
	bool Terminated(void) { return bTerminated; }
};

//==========================================================================
//
//	Cnversation UI windows
//
//==========================================================================

//
//	KFilledRectWindow
//
class KFilledRectWindow:public KWindow
{
	DECLARE_CLASS(KFilledRectWindow, KWindow, 0);
	NO_DEFAULT_CONSTRUCTOR(KFilledRectWindow);

	void DrawWindow(KGC *gc);
};

//
//	KConChoiceWindow
//
class KConChoiceWindow:public KButtonWindow
{
	DECLARE_CLASS(KConChoiceWindow, KButtonWindow, 0);
	NO_DEFAULT_CONSTRUCTOR(KConChoiceWindow);

	KConChoice *Choice;

	void FocusEnteredWindow(void);
	void FocusLeftWindow(void);
};

//
//	KConWindow
//
class KConWindow:public KModalWindow
{
	DECLARE_CLASS(KConWindow, KModalWindow, 0);

	KConWindow();
	void InitWindow(void);
	void CleanUp(void);

protected:
	KConPlay *ConPlay;
	KFilledRectWindow *UpperConWindow;
	KFilledRectWindow *LowerConWindow;
	KTextWindow *WinSpeech;
	KConChoiceWindow *WinChoices[10];
	int NumChoices;

public:
	void SetConPlay(KConPlay *NewPlay) { ConPlay = NewPlay; }
	void Clear(void);
	void DisplayText(const char *Text);
	void AppendText(const char *Text);
	void DisplayChoice(KConChoice *Choice);

	bool KeyPressed(int key);
	bool ButtonActivated(KWindow *button);
	void DescendantRemoved(KWindow *Child);

protected:
	void CreateSpeechWindow(void);
};

//
//	KConWindowFirst
//
class KConWindowFirst:public KModalWindow
{
	DECLARE_CLASS(KConWindowFirst, KModalWindow, 0);

	KConWindowFirst();
	void InitWindow(void);

protected:
	KConPlay *ConPlay;
	KFilledRectWindow *LowerConWindow;
	KTextWindow *WinSpeech;

public:
	void SetConPlay(KConPlay *NewPlay) { ConPlay = NewPlay; }
	void Clear(void);
	void DisplayText(const char *Text);
	void AppendText(const char *Text);

	void DescendantRemoved(KWindow *Child);
};
