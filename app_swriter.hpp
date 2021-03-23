#include <cstdio>
#include <string>
#include "shellexec.hpp"
#include "cmdtfunc.h"
using namespace std;

#ifndef _APP_SWRITER
#define _APP_SWRITER

/*
File formatz:

<0|1> [color] [char] = char
<1> [footnote] [align] = a line's end with an EOL
*/

struct swChar {
	char disChar;
	string disColor;
};

swChar createSC(char dChar, string dColor) {
	swChar nSwc;
	nSwc.disChar=dChar;
	nSwc.disColor=dColor;
	return nSwc;
}

#define DEFAULT_COLOR "07"

void printSC(swChar scObject) {
	setColor(scObject.disColor);
	printf("%c",scObject.disChar);
	setColor(DEFAULT_COLOR);
}

#define ALIGN_LEFT 'L'
#define ALIGN_MIDDLE 'M'
#define ALIGN_RIGHT 'R'

#define DEFAULT_ALIGN ALIGN_LEFT

struct swLine {
	vector<swChar> scline;
	char align;
	string footnote;
};

swLine createSW(vector<swChar> sscline, char salign, string sfootnote) {
	swLine nSwl;
	nSwl.scline=sscline;
	//nSwl.len=slen;
	nSwl.align=salign;
	nSwl.footnote=sfootnote;
	return nSwl;
} 

void rtPrint(int inf,int lwidth) {
	for (int i = 0; i < int(lwidth/2)-int(inf/2); i++) printf(" ");
	// then you can print something you like.
}

void raPrint(int inf,int lwidth) {
	//printf("%d\n",lwidth-inf);
	for (int i = 0; i < lwidth-inf; i++) printf(" ");
}

void makeAlign(char atype,int lwidth,int inf) {
	switch (atype) {
		case ALIGN_LEFT:
			break;
		case ALIGN_MIDDLE:
			rtPrint(inf,lwidth);
			break;
		case ALIGN_RIGHT:
		//	printf("RIGHT\n"); 
			raPrint(inf,lwidth);
			break;
		default:
			break;
	}
}

#define clearVec(vector_object) while(!vector_object.empty()) vector_object.pop_back()

#define NO_FOOTNOTE "No_footnote"

// returning modified.
string swAppMain(string fileinfo) {
	string cmd;
	string buf = "";
	vector<string> argsp;
	// readup
	vector<string> fsp;
	vector<swLine> fsw;
	vector<swChar> tfsc;
	fsp = spiltLines(fileinfo);
	int lchar = 0;
	for (vector<string>::iterator i = fsp.begin(); i != fsp.end(); i++) {
		int lntype;
		string lncolor;
		char lnc_input[10001];
		char lnchar = ' '; // It's works!
		sscanf(i->c_str(),"%d%s %c",&lntype,lnc_input,&lnchar);
		lncolor = lnc_input;
		switch (lntype) {
			case 0:
				lchar++;
				//printf("%c\n",lnchar);//here is a bug: can't input yet
				tfsc.push_back(createSC(lnchar,lncolor));
				// normal char
				break;
			case 1:
				fsw.push_back(createSW(tfsc,lnchar,lncolor));
				// statement
				lchar = 0;
				clearVec(tfsc);
				// endl
				break;
			default:
				printf("Document corruption\n");
				return ""; // corruption
		}
	}
	do {
		/*
q = exit and save
a [line] [info] = add between
p = add in bottom
m [line] = modify
d [line] = delete
v (line) = view (as unformatted lines)
s (line) = view (as formatted lines) 
g [line] <L|M|R> = alignment
c [line] [begin] [end] [color] = modify color (include B and E)
o [line] [color] = modify all line's color
n [line] = add footnote
e [line] = show footnote
*/
		printf("> ");//prompts
		cmd = getl();
		argsp = split_arg(cmd,false);
		clearVec(tfsc);
		if (argsp[0][0]=='q') break;
	//	int ipos;
		int lineid;
		string stmp;
		switch (argsp[0][0]) {
			case 'a':
				stmp = "";
				for (int i = 2; i < argsp.size(); i++) stmp = stmp + argsp[i] + " ";
				for (int i = 0; i < stmp.length(); i++) {
					tfsc.push_back(createSC(stmp[i],DEFAULT_COLOR));
				}
				fsw.insert(fsw.begin()+atoi(argsp[1].c_str()),createSW(tfsc,DEFAULT_ALIGN,NO_FOOTNOTE));
				break;
			case 'p':
				stmp = "";
				for (int i = 1; i < argsp.size(); i++) stmp = stmp + argsp[i] + " ";
				//printf("%s\n",stmp.c_str());
				for (int i = 0; i < stmp.length(); i++) {
					tfsc.push_back(createSC(stmp[i],DEFAULT_COLOR));
				}
				fsw.push_back(createSW(tfsc,DEFAULT_ALIGN,NO_FOOTNOTE));
				break;
			case 'm':
				stmp = "";
				for (int i = 2; i < argsp.size(); i++) stmp = stmp + argsp[i] + " ";
				for (int i = 0; i < stmp.length(); i++) {
					tfsc.push_back(createSC(stmp[i],DEFAULT_COLOR));
				}
				fsw[atoi(argsp[1].c_str())]=createSW(tfsc,DEFAULT_ALIGN,NO_FOOTNOTE);
				break;
			case 'd':
				fsw.erase(fsw.begin()+atoi(argsp[1].c_str()));
				break;
			case 'v':
				if (argsp.size() > 1) {
					for (vector<swChar>::iterator j = fsw[atoi(argsp[1].c_str())].scline.begin(); j != fsw[atoi(argsp[1].c_str())].scline.end(); j++) {
						printf("%c",j->disChar);
					}
					printf("\n");
					break;
				}
				lineid = 0;
				for (vector<swLine>::iterator i = fsw.begin(); i != fsw.end(); i++) {
					printf("%3d ",lineid++);
					for (vector<swChar>::iterator j = i->scline.begin(); j != i->scline.end(); j++) {
						printf("%c",j->disChar);
					}
					printf("\n");
				}
				break;
			case 's':
				if (argsp.size() > 1) {
				//	printf("%c\n",fsw[atoi(argsp[1].c_str())].align);
					makeAlign(fsw[atoi(argsp[1].c_str())].align,SCREEN_WIDTH,fsw[atoi(argsp[1].c_str())].scline.size());
					for (vector<swChar>::iterator j = fsw[atoi(argsp[1].c_str())].scline.begin(); j != fsw[atoi(argsp[1].c_str())].scline.end(); j++) {
						printSC(*j);
					}
					printf("\n");
					break;
				}
				for (vector<swLine>::iterator i = fsw.begin(); i != fsw.end(); i++) {
					makeAlign(i->align,SCREEN_WIDTH,i->scline.size());
					for (vector<swChar>::iterator j = i->scline.begin(); j != i->scline.end(); j++) {
						printSC(*j);
					}
					printf("\n");
				}
				break;
			case 'g':
				fsw[atoi(argsp[1].c_str())].align=argsp[2][0];
				break;
			case 'c':
				for (int i = atoi(argsp[2].c_str()); i <= atoi(argsp[3].c_str()); i++) {
					fsw[atoi(argsp[1].c_str())].scline[i].disColor=argsp[4];
				}
				break;
			case 'o':
				//printf("%d\n%d\n",atoi(argsp[1].c_str()),fsw[atoi(argsp[1].c_str())].scline.size());
				for (int i = 0; i < fsw[atoi(argsp[1].c_str())].scline.size(); i++) {
					fsw[atoi(argsp[1].c_str())].scline[i].disColor=argsp[2];
				}
				break;
			case 'n':
				stmp = argsp[2];//column not allowed
				//for (int i = 2; i < argsp.size(); i++) stmp = stmp + argsp[i] + " ";
				fsw[atoi(argsp[1].c_str())].footnote = stmp;
				break;
			case 'e':
				printf("%s\n",fsw[atoi(argsp[1].c_str())].footnote.c_str());
				break;
		} 
	} while (true);
	// save things
	for (vector<swLine>::iterator i = fsw.begin(); i != fsw.end(); i++) {
		for (vector<swChar>::iterator j = i->scline.begin(); j != i->scline.end(); j++) {
			buf = buf + "0 " + j->disColor + " " + j->disChar + "\n";
		}
		buf = buf + "1 " + i->footnote + " " + i->align + "\n";
	}
	return buf;
}

#endif
