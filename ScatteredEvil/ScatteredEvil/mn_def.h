// Menu defines and types.

#ifndef __MENU_DEFS_H_
#define __MENU_DEFS_H_

// Forward declarations
class KCanvas;
class KGC;
class	KWindow;

#include "KCanvas.h"
#include "KGC.h"
#include "KWindow.h"

// Macros

#define LEFT_DIR 0
#define RIGHT_DIR 1
#define ITEM_HEIGHT 20
#define SELECTOR_XOFFSET (-28)
#define SELECTOR_YOFFSET (-1)
#define SLOTTEXTLEN	16
#define ASCII_CURSOR '['

// Types

class KMenuWindow:public KWindow
{
public:
	KMenuWindow(KWindow *InParent) : KWindow(InParent) { }
};

typedef enum
{
	ITT_EMPTY,
	ITT_EFUNC,
	ITT_LRFUNC,
	ITT_SETMENU,
	ITT_INERT
} ItemType_t;

typedef enum
{
	MENU_MAIN,
	MENU_CLASS,
	MENU_SKILL,
	MENU_CHAR,
	MENU_OPTIONS,
	MENU_OPTIONS2,
	MENU_GAMEPLAY,
	MENU_GRAPHICS,
	MENU_EFFECTS,
	MENU_RESOLUTION,
	MENU_CONTROLS,
	MENU_MOUSEOPTS,
	MENU_JOYCONFIG,
	MENU_FILES,
	MENU_LOAD,
	MENU_SAVE,
	MENU_NONE
} MenuType_t;

typedef struct
{
	ItemType_t type;
	char *text;
	void (*func)(int option);
	int option;
	MenuType_t menu;
} MenuItem_t;

class Menu_t:public KMenuWindow
{
public:
	int x;
	int y;
	void (*drawFunc)(void);
	int itemCount;
	MenuItem_t *items;
	int oldItPos;
	MenuType_t prevMenu;
	// Enhancements. -jk
	void (*textDrawer)(char*,int,int);
	int	itemHeight;
	// For scrolling menus.
	int firstItem, numVisItems;
	float offset; // To y.

	//	Constructor
	Menu_t(int Ax, int Ay, void (*AdrawFunc)(void),
		int AitemCount, MenuItem_t *Aitems,
		int AoldItPos, MenuType_t AprevMenu,
		void (*AtextDrawer)(char*,int,int),
		int	AitemHeight,
		int AfirstItem, int AnumVisItems,
		float Aoffset = 0) : KMenuWindow(NULL),
		x(Ax), y(Ay), drawFunc(AdrawFunc), itemCount(AitemCount), items(Aitems),
		oldItPos(AoldItPos), prevMenu(AprevMenu),
		textDrawer(AtextDrawer), itemHeight(AitemHeight),
		firstItem(AfirstItem), numVisItems(AnumVisItems), offset(Aoffset)
	{
	}

	void DrawWindow(KGC *)
	{
		if (drawFunc)
		{
			drawFunc();
		}
	}
};

extern boolean shiftdown;

#endif // __MENU_DEFS_H_