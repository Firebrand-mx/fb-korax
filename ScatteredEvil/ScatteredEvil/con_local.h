#include "mn_def.h"

enum EConEventType
{
	ET_Speech,
	ET_Choice,
	ET_End
};

struct KConversation;

//
//	KConSpeech
//
struct KConSpeech
{
	char *Speech;
};

//
//	KConChoice
//
struct KConChoice
{
	char *ChoiceText;			// Choice text
	char *ChoiceLabel;			// Choice label

	bool bDisplayAsSpeech;		// Display choice as speech after user selects it

	KConChoice *NextChoice;		// Pointer to next choice
};

//
//	KConEvent
//
struct KConEvent
{
	EConEventType EventType;	// Event type
	char *Label;				// Optional event label
	KConEvent *NextEvent;		// Pointer to next event
	KConversation *Conversation;// Conversation that owns this event
};

//
//	KConversation
//
struct KConversation
{
	char *ConName;				// Conversation name
	char *ConOwnerName;			// Conversation owner name
	//bool bFirstPerson;		// Remain in First-Person mode
	//bool bRandomCamera;		// Random camera placement (can be overriden)

	KConEvent *EventList;		// First event

	int ConID;					// Internal conversation ID

	KConversation *NextCon;		// Pointer to next conversation
};

//
//	KConEventSpeech
//
struct KConEventSpeech:KConEvent
{
	mobj_t *Speaker;			// Mobj who speaks
	char *SpeakerName;			// Mobj name
	mobj_t *SpeakingTo;			// Mobj who is being spoken to
	char SpeakingToName;		// Mobj name
	KConSpeech *ConSpeech;		// Speech
	bool bContinued;			// True if this speech continued from last speech event
};

//
//	KConEventChoice
//
struct KConEventChoice:KConEvent
{
	bool bClearScreen;
	KConChoice *ChoiceList;
};

//
//	KConEventEnd
//
struct KConEventEnd:KConEvent
{
};

//==========================================================================
//
//	Playback of cnversations
//
//==========================================================================

class KConPlay:public KObject
{
	DECLARE_CLASS(KConPlay, KObject, 0);
	NO_DEFAULT_CONSTRUCTOR(KConPlay);

protected:
	bool bConversationStarted;
	bool bTerminated;

	mobj_t *ConOwner;
	mobj_t *ConTarget;

	class KConWindow *ConWin;

	mobj_t *CurrentSpeaker;
	mobj_t *CurrentSpeakingTo;

	int con_timer;
	int test_state;

public:
	bool StartConversation(mobj_t *NewOwner, mobj_t *NewTarget);
	void TerminateConversation(void);

	void Tick(void);

	void SetCameraMobjs(void);

	void PlayNextEvent(void);
	void PlayChoice(KConChoice *Choice);

	bool ConversationStarted(void) { return bConversationStarted; }
	bool Terminated(void) { return bTerminated; }
};

//==========================================================================
//
//	Cnversation UI windows
//
//==========================================================================

//
//	KConChoiceWindow
//
class KConChoiceWindow:public KWindow
{
	DECLARE_CLASS(KConChoiceWindow, KWindow, 0);
	KConChoiceWindow();

	KConChoice *Choice;
	KTextWindow *WinText;

	void InitWindow(void);

	void SetText(const char *Text);

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

protected:
	KConPlay *ConPlay;
	KTextWindow *WinSpeech;
	KConChoiceWindow *WinChoices[10];
	int NumChoices;
	int CurrentChoice;

public:
	void SetConPlay(KConPlay *NewPlay) { ConPlay = NewPlay; }
	void Clear(void);
	void DisplayText(const char *Text);
	void DisplayChoice(KConChoice *Choice);

	bool KeyPressed(int key);
	void DescendantRemoved(KWindow *Child);

protected:
	void CreateSpeechWindow(void);
};
