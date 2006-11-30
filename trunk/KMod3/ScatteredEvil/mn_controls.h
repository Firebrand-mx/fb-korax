// Control flags.
#define CLF_ACTION		0x1		// The control is an action (+/- in front).
#define CLF_REPEAT		0x2		// Bind down + repeat.

struct Control_t
{
	char		*command;		// The command to execute.
	int			flags;
	int			defKey;			// 
	int			defMouse;		// Zero means there is no default.
	int			defJoy;			//
};

//
// !!! Add new controls to the end, the existing indices must remain unchanged !!!
//
static Control_t controls[] =
{
	// Actions (must be first so the H2A_* constants can be used).
	"left",			CLF_ACTION,		DDKEY_LEFTARROW, 0, 0,
	"right",		CLF_ACTION,		DDKEY_RIGHTARROW, 0, 0,
	"forward",		CLF_ACTION,		'w', 0, 0,
	"backward",		CLF_ACTION,		's', 0, 0,
	"strafel",		CLF_ACTION,		'a', 0, 0,
	"strafer",		CLF_ACTION,		'd', 0, 0,
	"jump",			CLF_ACTION,		' ', 0, 5,
	"fire",			CLF_ACTION,		DDKEY_RCTRL, 1, 1,
	"use",			CLF_ACTION,		'e', 2, 4,
	"strafe",		CLF_ACTION,		DDKEY_RALT, 3, 2,
	
	"speed",		CLF_ACTION,		DDKEY_RSHIFT, 0, 3,
	"flyup",		CLF_ACTION,		DDKEY_PGUP, 0, 8,
	"flydown",		CLF_ACTION,		DDKEY_INS, 0, 9,
	"falldown",		CLF_ACTION,		DDKEY_HOME, 0, 0,
	"lookup",		CLF_ACTION,		DDKEY_PGDN, 0, 6,
	"lookdown",		CLF_ACTION,		DDKEY_DEL, 0, 7,
	"lookcntr",		CLF_ACTION,		DDKEY_END, 0, 0,
	"usearti",		CLF_ACTION,		DDKEY_ENTER, 0, 0,
	"mlook",		CLF_ACTION,		'm', 0, 0,
	"jlook",		CLF_ACTION,		'j', 0, 0,
	
	"nextwpn",		CLF_ACTION,		0, DDMB_MWHEELDOWN, 0,
	"prevwpn",		CLF_ACTION,		0, DDMB_MWHEELUP, 0,
	"weapon1",		CLF_ACTION,		'1', 0, 0,
	"weapon2",		CLF_ACTION,		'2', 0, 0,
	"weapon3",		CLF_ACTION,		'3', 0, 0,
	"weapon4",		CLF_ACTION,		'4', 0, 0,
	"panic",		CLF_ACTION,		DDKEY_BACKSPACE, 0, 0,
	"torch",		CLF_ACTION,		0, 0, 0,
	"health",		CLF_ACTION,		'\\', 0, 0,
	"mystic",		CLF_ACTION,		0, 0, 0,
	
	"krater",		CLF_ACTION,		0, 0, 0,
	"spdboots",		CLF_ACTION,		0, 0, 0,
	"blast",		CLF_ACTION,		'9', 0, 0,
	"teleport",		CLF_ACTION,		'8', 0, 0,
	"teleothr",		CLF_ACTION,		'7', 0, 0,
	"poison",		CLF_ACTION,		'0', 0, 0,
	"cantdie",		CLF_ACTION,		'5', 0, 0,
	"servant",		CLF_ACTION,		0, 0, 0,
	"egg",			CLF_ACTION,		'6', 0, 0,
	"demostop",		CLF_ACTION,		'o', 0, 0,
	"duck",		    CLF_ACTION,		'c', 0, 0,
	"spell1",		CLF_ACTION,		DDKEY_NUMPAD1, 0, 0,
	"spell2",		CLF_ACTION,		DDKEY_NUMPAD2, 0, 0,
	"spell3",		CLF_ACTION,		DDKEY_NUMPAD3, 0, 0,
	"spell4",		CLF_ACTION,		DDKEY_NUMPAD4, 0, 0,
	"spell5",		CLF_ACTION,		DDKEY_NUMPAD5, 0, 0,
	"spell6",		CLF_ACTION,		DDKEY_NUMPAD6, 0, 0,
	"spell7",		CLF_ACTION,		DDKEY_NUMPAD7, 0, 0,
	"spell8",		CLF_ACTION,		DDKEY_NUMPAD8, 0, 0,
/*	"sell",		    CLF_ACTION,		0, 0, 0,
	"reply",	    CLF_ACTION,		0, 0, 0,*/

	// Menu hotkeys (default: F1 - F12).
	"infoscreen",	0,				DDKEY_F1, 0, 0,
	"loadgame",		0,				DDKEY_F2, 0, 0,
	"savegame",		0,				DDKEY_F3, 0, 0,
	"soundmenu",	0,				DDKEY_F4, 0, 0,
	"suicide",		0,				DDKEY_F5, 0, 0,
	"quicksave",	0,				DDKEY_F6, 0, 0,
	"quickload",	0,				DDKEY_F9, 0, 0,
	"quit",			0,				DDKEY_F10, 0, 0,
	"togglegamma",	0,				DDKEY_F11, 0, 0,
	"spy",			0,				DDKEY_F12, 0, 0,

	// Inventory.
	"invleft",		CLF_REPEAT,		'[', 0, 0,
	"invright",		CLF_REPEAT,		']', 0, 0,

	// Screen controls.
	"viewsize +",	CLF_REPEAT,		'=', 0, 0,
	"viewsize -",	CLF_REPEAT,		'-', 0, 0,

	// Misc.
	"pause",		0,				'p', 0, 0,
	"",				0,				0, 0, 0
};

//==========================================================================
//
//	KMenuChoiceControl
//
//==========================================================================

class KMenuChoiceControl:public KMenuItem_t
{
	DECLARE_CLASS(KMenuChoiceControl, KMenuItem_t, 0);

	char *ControlText;
	int ControlIndex;
	bool bGrabbing;

	KMenuChoiceControl(void)
	{
		Font = KCanvas::SmallFont;
	}

	bool KeyPressed(int key)
	{
		switch (key)
		{
		case DDKEY_ENTER:
			bGrabbing = true;
			S_StartSound(NULL, SFX_DOOR_LIGHT_CLOSE);
			return true;
		}
		return Super::KeyPressed(key);
	}

	void DrawWindow(KGC *gc)
	{
		char		controlCmd[80];
		char		buff[80], prbuff[80], *token;
		Control_t	*ctrl;

		gc->DrawText(0, 0, ControlText);

		ctrl = controls + ControlIndex;
		strcpy(buff, "");
		if (ctrl->flags & CLF_ACTION)
			sprintf(controlCmd, "+%s", ctrl->command);
		else
			strcpy(controlCmd, ctrl->command);
		// Let's gather all the bindings for this command.
		if (!B_BindingsForCommand(controlCmd, buff))
			strcpy(buff, "NONE");

		// Now we must interpret what the bindings string says.
		// It may contain characters we can't print.
		strcpy(prbuff, "");
		token = strtok(buff, " ");
		while (token)
		{
			if (token[0] == '+')
				spacecat(prbuff, token+1);
			if (token[0] == '*' && !(ctrl->flags & CLF_REPEAT) || token[0] == '-')
				spacecat(prbuff, token);
			token = strtok(NULL, " ");
		}

		if (bGrabbing)
		{
			// We're grabbing for this control.
			spacecat(prbuff, "...");
		}

		gc->SetFont(KCanvas::YellowFont);
		gc->DrawText(134, 0, prbuff);
	}

	bool RawInputEvent(event_t *event)
	{
		// We're interested in key or button down events.
		if (bGrabbing && (event->type == ev_keydown || event->type == ev_mousebdown 
			|| event->type == ev_joybdown || event->type == ev_povdown))
		{
			// We'll grab this event.
			char cmd[256], buff[256], evname[80];
			boolean del = false;
			Control_t	*ctrl;

			ctrl = controls + ControlIndex;
			// Check for a cancel.
			if (event->type == ev_keydown)
			{
				if (event->data1 == '`') // Tilde clears everything.
				{
					if (ctrl->flags & CLF_ACTION)
						sprintf(cmd, "delbind +%s -%s", ctrl->command,
							ctrl->command);
					else
						sprintf(cmd, "delbind \"%s\"", ctrl->command);
					CON_Execute(cmd, true);
					bGrabbing = false;
					return true;
				}
				else if(event->data1 == DDKEY_ESCAPE)
				{
					bGrabbing = false;
					return true;
				}
			}

			// We shall issue a silent console command, but first we need
			// a textual representation of the event.
			B_EventConverter(evname, event, false); // "Deconstruct" into a name.

			// If this binding already exists, remove it.
			sprintf(cmd, "%s%s", ctrl->flags & CLF_ACTION? "+" : "",
				ctrl->command);
			if (B_BindingsForCommand(cmd, buff))
				if (findtoken(buff, evname, " "))		// Get rid of it?
				{
					del = true;
					strcpy(buff, "");
				}
			if (!del) sprintf(buff, "\"%s\"", ctrl->command);
			sprintf(cmd, "%s %s %s", ctrl->flags & CLF_REPEAT? "bindr" : "bind",
				evname+1, buff);
			CON_Execute(cmd, true);
			// We've finished the grab.
			bGrabbing = false;
			S_StartSound(NULL, SFX_CHAT);
			return true;
		}
		return Super::RawInputEvent(event);
	}

private:
	static void spacecat(char *str, const char *catstr)
	{
		if(str[0]) strcat(str, " ");
	
		// Also do some filtering.
		switch(catstr[0])
		{
		case '\\':
			strcat(str, "bkslash");
			break;
	
		case '[':
			strcat(str, "sqbtopen");
			break;

		case ']':
			strcat(str, "sqbtclose");
			break;

		default:
			strcat(str, catstr);
		}
	}

	static int findtoken(char *string, char *token, char *delim)
	{
		char *ptr = strtok(string, delim);
		while(ptr)
		{
			if(!stricmp(ptr, token)) return true;
			ptr = strtok(NULL, delim);
		}
		return false;
	}
};
IMPLEMENT_CLASS(KMenuChoiceControl);

//==========================================================================
//
//	KMenuControlsLabel
//
//==========================================================================

class KMenuControlsLabel:public KMenuItem_t
{
	DECLARE_CLASS(KMenuControlsLabel, KMenuItem_t, 0);

	char *LabelText;

	KMenuControlsLabel(void)
	{
		Font = KCanvas::YellowFont;
	}

	void DrawWindow(KGC *gc)
	{
		if (LabelText)
		{
			gc->DrawText(0, 0, LabelText);
		}
	}
};
IMPLEMENT_CLASS(KMenuControlsLabel);

//==========================================================================
//
//	KMenuScreenControls
//
//==========================================================================

class KMenuScreenControls:public KMenuScreen
{
	DECLARE_CLASS(KMenuScreenControls, KMenuScreen, 0);

	int CurX;
	int CurY;

	KMenuScreenControls(void)
	{
		ChoiceStartX = 32;
		ChoiceStartY = 26;
		itemHeight = 9;
		Font = KCanvas::SmallFont;
	}

	void CreateChoices(void)
	{
		CurX = ChoiceStartX;
		CurY = ChoiceStartY;
		CreateItem("PLAYER ACTIONS");
		AddControl("LEFT :", H2A_TURNLEFT);
		AddControl("RIGHT :", H2A_TURNRIGHT);
		AddControl("FORWARD :", H2A_FORWARD);
		AddControl("BACKWARD :", H2A_BACKWARD);
		AddControl("STRAFE LEFT :", H2A_STRAFELEFT);
		AddControl("STRAFE RIGHT :", H2A_STRAFERIGHT);
		AddControl("JUMP :", H2A_JUMP);
		AddControl("DUCK :", H2A_DUCK);
		AddControl("FIRE :", H2A_FIRE);
		AddControl("USE :", H2A_USE);
		AddControl("STRAFE :", H2A_STRAFE);
		AddControl("SPEED :", H2A_SPEED);
		AddControl("FLY UP :", H2A_FLYUP);
		AddControl("FLY DOWN :", H2A_FLYDOWN);
		AddControl("FALL DOWN :", H2A_FLYCENTER);
		AddControl("LOOK UP :", H2A_LOOKUP);
		AddControl("LOOK DOWN :", H2A_LOOKDOWN);
		AddControl("LOOK CENTER :", H2A_LOOKCENTER);
		AddControl("MOUSE LOOK :", H2A_MLOOK);
		AddControl("JOYSTICK LOOK :", H2A_JLOOK);
		AddControl("NEXT WEAPON :", H2A_NEXTWEAPON);
		AddControl("PREV WEAPON :", H2A_PREVIOUSWEAPON);
		AddControl("WEAPON 1 :", H2A_WEAPON1);
		AddControl("WEAPON 2 :", H2A_WEAPON2);
		AddControl("WEAPON 3 :", H2A_WEAPON3);
		AddControl("WEAPON 4 :", H2A_WEAPON4);
		AddControl("PANIC :", H2A_PANIC);
		AddControl("SPELL1 :", H2A_SPELL1);
		AddControl("SPELL2 :", H2A_SPELL2);
		AddControl("SPELL3 :", H2A_SPELL3);
		AddControl("SPELL4 :", H2A_SPELL4);
		AddControl("SPELL5 :", H2A_SPELL5);
		AddControl("SPELL6 :", H2A_SPELL6);
		AddControl("SPELL7 :", H2A_SPELL7);
/*		AddControl("SPELL8 :", H2A_SPELL8);*/

		CurX = ChoiceStartX + 320;
		CurY = ChoiceStartY;
		CreateItem("ARTIFACTS");
		AddControl("TORCH :", H2A_TORCH);
		AddControl("QUARTZ FLASK :", H2A_HEALTH);
		AddControl("MYSTIC URN :", H2A_MYSTICURN);
		AddControl("KRATER OF MIGHT :", H2A_KRATER);
		AddControl("BOOTS OF SPEED :", H2A_SPEEDBOOTS);
		AddControl("REPULSION :", H2A_BLASTRADIUS);
		AddControl("CHAOS DEVICE :", H2A_TELEPORT);
		AddControl("BANISHMENT :", H2A_TELEPORTOTHER);
		AddControl("BOOTS OF SPEED :", H2A_SPEEDBOOTS);
		AddControl("FLECHETTE :", H2A_POISONBAG);
		AddControl("DEFENDER :", H2A_INVULNERABILITY);
		AddControl("DARK SERVANT :", H2A_DARKSERVANT);
		AddControl("PORKELATOR :", H2A_EGG);
		CreateItem(NULL);
		CreateItem("INVENTORY");
		AddControl("INVENTORY LEFT :", NUM_ACTIONS+10);
		AddControl("INVENTORY RIGHT :", NUM_ACTIONS+11);
		AddControl("USE ARTIFACT :", H2A_USEARTIFACT);
		CreateItem(NULL);
		CreateItem("MENU HOTKEYS");
		AddControl("INFO :", NUM_ACTIONS);
		AddControl("SOUND MENU :", NUM_ACTIONS+3);
		AddControl("LOAD GAME :", NUM_ACTIONS+1);
		AddControl("SAVE GAME :", NUM_ACTIONS+2);
		AddControl("QUICK LOAD :", NUM_ACTIONS+6);
		AddControl("QUICK SAVE :", NUM_ACTIONS+5);
		AddControl("SUICIDE :", NUM_ACTIONS+4);
//		AddControl("END GAME :", NUM_ACTIONS+6);
		AddControl("QUIT :", NUM_ACTIONS+7);
//		AddControl("MESSAGES ON/OFF:", NUM_ACTIONS+7);
		AddControl("GAMMA CORRECTION :", NUM_ACTIONS+8);
		AddControl("SPY MODE :", NUM_ACTIONS+9);
		CreateItem(NULL);
		CreateItem("SCREEN");
		AddControl("SMALLER VIEW :", NUM_ACTIONS+13);
		AddControl("LARGER VIEW :", NUM_ACTIONS+12);
		CreateItem(NULL);
		CreateItem("MISCELLANEOUS");
		AddControl("STOP DEMO :", H2A_STOPDEMO);
		AddControl("PAUSE :", NUM_ACTIONS+14);
/*		AddControl("DUCK :", H2A_DUCK);
		AddControl("SELL :", H2A_SELL);
		AddControl("REPLY :", H2A_REPLY);*/
	}

	void CreateItem(char *text)
	{
		KMenuControlsLabel *It = NewWindow(KMenuControlsLabel, this);
		It->LabelText = text;
		It->SetPos(CurX, CurY);
		It->Disable();
		CurY += itemHeight;
	}

	void AddControl(char *text, int option)
	{
		KMenuChoiceControl *It;
	
		It = NewWindow(KMenuChoiceControl, this);
		It->SetPos(CurX, CurY);
		It->ControlText = text;
		It->ControlIndex = option;
		CurY += itemHeight;
	}

	void KMenuScreenControls::DrawWindow(KGC *gc)
	{
		gc->SetFont(KCanvas::BigFont);
		gc->DrawText(280, 4, "CONTROLS");
	}
};
IMPLEMENT_CLASS(KMenuScreenControls);

//==========================================================================
//
//	H2_DefaultBindings
//
//	Set default bindings for unbound Controls.
//
//==========================================================================

void H2_DefaultBindings(void)
{
	int			i;
	Control_t	*ctr;
	char		evname[80], cmd[256], buff[256];
	event_t		event;
	
	// Check all Controls.
	for(i=0; controls[i].command[0]; i++)
	{
		ctr = controls + i;
		// If this command is bound to something, skip it.
		sprintf(cmd, "%s%s", ctr->flags & CLF_ACTION? "+" : "",
			ctr->command);
		if (B_BindingsForCommand(cmd, buff)) continue;

		// This Control has no bindings, set it to the default.
		sprintf(buff, "\"%s\"", ctr->command);
		if(ctr->defKey)
		{
			event.type = ev_keydown;
			event.data1 = ctr->defKey;
			B_EventConverter(evname, &event, false);
			sprintf(cmd, "%s %s %s", ctr->flags & CLF_REPEAT? "safebindr" : "safebind",
				evname+1, buff);
			CON_Execute(cmd, true);
		}
		if(ctr->defMouse)
		{
			event.type = ev_mousebdown;
			event.data1 = 1 << (ctr->defMouse-1);
			B_EventConverter(evname, &event, false);
			sprintf(cmd, "%s %s %s", ctr->flags & CLF_REPEAT? "safebindr" : "safebind",
				evname+1, buff);
			CON_Execute(cmd, true);
		}
		if(ctr->defJoy)
		{
			event.type = ev_joybdown;
			event.data1 = 1 << (ctr->defJoy-1);
			B_EventConverter(evname, &event, false);
			sprintf(cmd, "%s %s %s", ctr->flags & CLF_REPEAT? "safebindr" : "safebind",
				evname+1, buff);
			CON_Execute(cmd, true);
		}
	}
}
