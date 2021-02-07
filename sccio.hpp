#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <conio.h> 
#include "cmdtfunc.h"
using namespace std;

#ifndef _SC_SCIO
#define _SC_SCIO 1

// it is useful ~
#ifndef nothing
void nothing(void) {
	return;//we must do something...such as returning
}
#endif

typedef void(*menufunc)(void);

struct menuitem {
	string menu_info;
	menufunc menu_act;
	bool menu_enable;
};

#ifndef KEYLOCKS

#define KEYLOCKS 1
#define UPROLL_KEY1 'W'
#define UPROLL_KEY2 'w'
#define DNROLL_KEY1 'S'
#define DNROLL_KEY2 's'
#define CONFIRM_KEY1 ' '

// i.e. ENTER
#define CONFIRM_KEY2 13

#endif

#ifndef CLEANUP_COMMAND

#define CLEANUP_COMMAND "cls"

#endif

// Let us do something fun

menuitem createMenuItem(string menu_info, menufunc menu_act, bool menu_enable) {
	menuitem m;
	m.menu_act=menu_act;
	m.menu_enable=menu_enable;
	m.menu_info=menu_info;
	return m;
}

vector<menuitem> operator << (vector<menuitem> basics_stream, string menuItemName) {
	vector<menuitem> new_stream;
	new_stream = basics_stream;
	new_stream.push_back(createMenuItem(menuItemName,nothing,true));
	return new_stream;
}

vector<menuitem> operator << (vector<menuitem> basics_stream, menuitem newMenuItem) {
	vector<menuitem> new_stream;
	new_stream = basics_stream;
	new_stream.push_back(newMenuItem);
	return new_stream;
}

// enough ...
int displayMenuItem(vector<menuitem> menuitemlist, string selectedMenuColor, string unselectedMenuColor, string disabledMenuColor, string entirePrompt) {
	int selected = 0; // always exists ~
	if (menuitemlist.size()==0) return -1;//fucku
	do {
		system(CLEANUP_COMMAND); 
		cout << entirePrompt << endl;
		for (int i = 0; i < menuitemlist.size(); i++) {
			if (i==selected) {
				setColor(selectedMenuColor);
			} else {
				setColor(unselectedMenuColor);
			}
			if (menuitemlist[i].menu_enable==false) {
				setColor(disabledMenuColor);
			}
			cout << menuitemlist[i].menu_info << endl;
			setColor("07");
		}
		int sold;
		switch (getch()) {
				case int(UPROLL_KEY1): case int(UPROLL_KEY2):
					sold = selected;
					if (selected>0) selected--;
					while (selected>0 && menuitemlist[selected].menu_enable==false) selected--;
					if (menuitemlist[selected].menu_enable==false) selected = sold;
					break;
				case int(DNROLL_KEY1): case int(DNROLL_KEY2):
					sold = selected;
					if ((selected+1)<menuitemlist.size()) selected++;
					while ((selected+1)<menuitemlist.size() && menuitemlist[selected].menu_enable==false) selected++;
					if (menuitemlist[selected].menu_enable==false) selected = sold;
					break;
				case int(CONFIRM_KEY1): case int(CONFIRM_KEY2):
					menuitemlist[selected].menu_act();
					return selected;
					break;
				default:
					break;
			}
	} while (1);
}


#endif 
