#include <iostream>
#include <cstdlib>
#include <string>
#include <windows.h>
#include <conio.h>
using namespace std;

#ifndef _CMDTFUNC_
#define _CMDTFUNC_

#define sHnd GetStdHandle(STD_OUTPUT_HANDLE)

#define FOREGROUND_YELLOW FOREGROUND_RED | FOREGROUND_GREEN
#define FOREGROUND_PINK FOREGROUND_RED | FOREGROUND_BLUE
#define FOREGROUND_CYAN FOREGROUND_GREEN | FOREGROUND_BLUE
#define FOREGROUND_WHITE FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE

#define BACKGROUND_YELLOW BACKGROUND_RED | BACKGROUND_GREEN
#define BACKGROUND_PINK BACKGROUND_RED | BACKGROUND_BLUE
#define BACKGROUND_CYAN BACKGROUND_GREEN | BACKGROUND_BLUE
#define BACKGROUND_WHITE BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE

#define rpush(val) result = result | val

#define ERR_INVAILD_COLOR 2 
#define ERR_INVAILD_COLOR_CHAR 3

void err(string info,int errcode) {
	cout<<info<<endl;
	exit(errcode);
	return;
}

WORD getColorByString(string s) {
	char b=toupper(s[0]),f=toupper(s[1]);
	WORD result = 0;
	switch (f) {
		case '0':
			break;
		case '9':
			rpush(FOREGROUND_INTENSITY);
		case '1':
			rpush(FOREGROUND_BLUE);
			break;
		case 'A':
			rpush(FOREGROUND_INTENSITY);
		case '2':
			rpush(FOREGROUND_GREEN);
			break;
		case 'B':
			rpush(FOREGROUND_INTENSITY);
		case '3':
			rpush(FOREGROUND_CYAN);
			break;
		case 'C':
			rpush(FOREGROUND_INTENSITY);
		case '4':
			rpush(FOREGROUND_RED);
			break;
		case 'D':
			rpush(FOREGROUND_INTENSITY);
		case '5':
			rpush(FOREGROUND_PINK);
			break;
		case 'E':
			rpush(FOREGROUND_INTENSITY);
		case '6':
			rpush(FOREGROUND_YELLOW);
			break;
		case 'F':
			rpush(FOREGROUND_INTENSITY);
		case '7':
			rpush(FOREGROUND_WHITE);
			break;
		case '8':
			err("Invaild color\n",ERR_INVAILD_COLOR);
			break;
		default:
			err("Invaild color\n",ERR_INVAILD_COLOR_CHAR);
			break;	
	}
	switch (b) {
		case '0':
			break;
		case '1':
			rpush(BACKGROUND_BLUE);
		case '9':
			rpush(BACKGROUND_INTENSITY);
			break;
		case '2':
			rpush(BACKGROUND_GREEN);
		case 'A':
			rpush(BACKGROUND_INTENSITY);
			break;
		case '3':
			rpush(BACKGROUND_CYAN);
		case 'B':
			rpush(BACKGROUND_INTENSITY);
			break;
		case '4':
			rpush(BACKGROUND_RED);
		case 'C':
			rpush(BACKGROUND_INTENSITY);
			break;
		case '5':
			rpush(BACKGROUND_PINK);
		case 'D':
			rpush(BACKGROUND_INTENSITY);
			break;
		case '6':
			rpush(BACKGROUND_YELLOW);
		case 'E':
			rpush(BACKGROUND_INTENSITY);
			break;
		case '7':
			rpush(BACKGROUND_WHITE);
		case 'F':
			rpush(BACKGROUND_INTENSITY);
			break;
		case '8':
			err("Invaild color\n",ERR_INVAILD_COLOR);
			break;
		default:
			err("Invaild color\n",ERR_INVAILD_COLOR_CHAR);
			break;	
	}
	return result;
}

void setColor(string s2) {
	SetConsoleTextAttribute(sHnd,getColorByString(s2));
}

#endif 
