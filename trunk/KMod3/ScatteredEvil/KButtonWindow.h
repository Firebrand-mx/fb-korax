
//**************************************************************************
//**
//** $RCSfile$
//** $Revision$
//** $Date$
//** $Author$
//**
//**************************************************************************

// Number of button states
#define MAX_BUTTON_STATES	6

// Button states are defined as follows:
// 0 - Sensitive,   unfocused, unpressed
// 1 - Sensitive,   unfocused, pressed
// 2 - Sensitive,   focused,   unpressed
// 3 - Sensitive,   focused,   pressed
// 4 - Insensitive, unfocused, unpressed
// 5 - Insensitive, unfocused, pressed
//
// (There are technically two more permutations, insensitive/focused
// pressed and unpressed, but insensitive widgets by definition cannot
// have focus.)

// KButtonDisplayInfo - Information for one button state's display
struct KButtonDisplayInfo
{
	KTexture	*Texture;
	FColor		TileColor;
	FColor		TextColor;
};

class KButtonWindow:public KTextWindow
{
	DECLARE_CLASS(KButtonWindow, KTextWindow, 0);

	// Structors
	KButtonWindow();
	//void CleanUp(void);

	// Button states
	bool bButtonPressed;	// True if button window is "activated"

protected:
	KTexture *CurTexture;	// Currently displayed texture
	FColor CurTileColor;	// Current tile color
	FColor CurTextColor;	// Current text color

private:
	// Button state information
	KButtonDisplayInfo Info[MAX_BUTTON_STATES];

public:
	// KButtonWindow interface
	void ActivateButton(int key);

	void SetButtonTextures(KTexture *normal=NULL,
	                       KTexture *pressed=NULL,
	                       KTexture *normalFocus=NULL,
	                       KTexture *pressedFocus=NULL,
	                       KTexture *normalInsensitive=NULL,
	                       KTexture *pressedInsensitive=NULL);

	void SetButtonColors(FColor normal=FColor(255,255,255),
	                     FColor pressed=FColor(255,255,255),
	                     FColor normalFocus=FColor(255,255,255),
	                     FColor pressedFocus=FColor(255,255,255),
	                     FColor normalInsensitive=FColor(255,255,255),
	                     FColor pressedInsensitive=FColor(255,255,255));

	void SetTextColors(FColor normal=FColor(255,255,255),
	                   FColor pressed=FColor(255,255,255),
	                   FColor normalFocus=FColor(255,255,255),
	                   FColor pressedFocus=FColor(255,255,255),
	                   FColor normalInsensitive=FColor(255,255,255),
	                   FColor pressedInsensitive=FColor(255,255,255));

	virtual void PressButton(int key);

	// KWindow interface callbacks
	void SensitivityChanged(bool bNewSensitivity);
	void FocusEnteredWindow(void);
	void FocusLeftWindow(void);
	void DrawWindow(KGC *gc);
	bool KeyPressed(int key);

protected:
	void ChangeButtonAppearance(void);
};
