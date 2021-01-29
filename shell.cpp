#include "filesystem.hpp"
#include "accounts.hpp"
#include "shellexec.hpp"
#include "cmdtfunc.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
using namespace std; 

funcall f;
acclist ac;
bool logged = false;
account curlogin;
int elevstack = 0,errval = 0;

string cdir = "/", env_user = "";
fdirnode *root = new fdirnode;

vector<string> empty_argv;

string getl(void) {
	string cmd = "";
	char c;
		while ((c=getchar())!='\n') {
			if (c=='\n') break;
			cmd = cmd + c;
		}
	return cmd;
}

/*
The commands will be add:

echo	Echo a message.
clear	Clear screen.
errval	Ouput previous return value.
ls		List folders and files. (-F for only folders, -f for only files.)
put		Output a line to file. (-o for override)
cat		View file.
mv		Move file.
cp		Copy file.
rm		Remove file.
cd		Switch 'current directory'.
color	Change command line output color.
whoami	User information.
logout	Logout. (*)
exit	Logout or logout elevated shell. (*)
elev	Make elevated permission. (*)
halt	Exit virtual shell. (*)
reboot	Reboot virtual shell. (*)
edit	A simple file editor.
*/
/*
Editor commands:
open (filename)
view [line]
add (content)
insert [line] (content) 
modify [line] (content)
delete [line]
duplicate [source line] [target line]
move [source line] [target line]
save
copy (new filename)
close
exit
*/
/*
vector<string> _ereadup(string filename) {
	vector<string> b;
	string dat = readFileA(root,cdir,argv[1]), b = "";
		for (int i = 0; i < dat.length(); i++) {
			if (dat[i]=='\n') {
				buf.push_back(b);
				b = "";
			} else {
				b = b + dat[i];
			}
		}
		if (b != "") buf.push_back(b);
	return b;
}*/
// A work in progress.
/*
int editor(int argc, vector<string> argv) {
	string fullname = "";
	vector<string> buf;
	vector<string> cmz;
	if (argc > 1) {
		fullname = cdir + argv[1];
		buf = _ereadup(fullname);
	}
	do {
		string cmd = getl();
		if (cmd == "open") {
			
		}
	} while (1);
}
*/

/*(continue)
svt		Virtual shell file tools (share files between host and app).
help	Show help message.

The shell prompt like:

[username]@[host name] [current directory]$ [inputting]

(*) means builtin command.
You can add your command, too.
*/

int echo(int argc, vector<string> argv) {
	if (argc < 2) return 1;
	vector<string>::iterator i;
	for (i = argv.begin()+1; i != argv.end(); i++) printf("%s ",(*i).c_str());
	printf("\n");
	return 0;
}

int put(int argc, vector<string> argv) {
	bool override = false;
	if (argc >= 2 && argv[1] == "-o") override = true;
	string dat = "", fn = argv[int(override)+1];
	if ((!override) && isFileExistsA(root,cdir,fn)) dat = readFileA(root,cdir,fn);
	if ((override&&argc>=4)||(argc>=3)) for (vector<string>::iterator i = argv.begin()+int(override)+2; i != argv.end(); i++) dat = dat + (*i) + " ";
	_proceedFile(resolve(cdir,root),fn,dat);
	return !isFileExistsA(root,"/",fn);
}



int _clear(int argc, vector<string> argv) {
	return system("cls");
}

int clear(void) {
	_clear(0,empty_argv);
}

int whoami(int argc, vector<string> argv) {
	printf("%s\n",env_user.c_str());
	return 0;
}

int errvala(int argc, vector<string> argv) {
	printf("%d\n",errval);
	return 0;
}

void _lso(vector<string> v) {
	int x = 0;
	for (vector<string>::iterator i = v.begin(); i != v.end(); i++) {
		x++;
		if (x>=3) {
			x=0;
			printf("%14s\n",(*i).c_str());
		} else {
			printf("%14s ",(*i).c_str());
		}
	}
}


int ls(int argc, vector<string> argv) {
	vector<string> v;
	bool flag = false;
	string dr = cdir;
	if (argc >= 2) {
		if (argc >= 3) dr = argv[2];
		if (argv[1]=="-F") {
			v = listFileA(root,dr,1);
			flag = true;
		} else if (argv[1]=="-f") {
			v = listFileA(root,dr,1);
			flag = true;
		} else {
			dr = argv[1];
		}
	}
	if (!flag) v = listFileA(root,dr,3);
	if (v.size()==0) {
		printf("Directory is empty\n");
		return 1;
	}
	_lso(v);
	printf("\n");
	return 0;
}

void initalize(void) {
	rootInit(root);
	ac=getAccounts();
	f["echo"]=echo;
	f["clear"]=_clear;
	f["whoami"]=whoami;
	f["errval"]=errvala;
	f["ls"]=ls;
	f["put"]=put;
}

void login(void) {
	// Login page
	string login_name, login_pwd;
	clear();
	printf("Seabird Galactic\nKernel 1.0.1.11 on unknown architecture\n\n");
	do {
		printf("%s Login: ",HOST_NAME);
		cin>>login_name;
		if (!ac.count(login_name)) {
			printf("Incorrect login name\n\n");
			continue;
		}
		if (ac[login_name].account_password == "") break;
		printf("Password: ");
		login_pwd = pwd_input();
		printf("\n");
		if (!(ac[login_name].account_password == login_pwd)) {
			printf("Incorrect password\n\n");
			continue;
		}
		break;
	} while (1);
	curlogin = ac[login_name];
	logged = true;
}

//char s[2049];

int shell(void) {
	printf("Welcome, %s\n",curlogin.account_name.c_str());
	env_user = curlogin.account_name;
	getchar();
	while (1) {
		string cmd = "";
		printf("%s@%s %s$ ",env_user.c_str(),HOST_NAME,cdir.c_str());
		// let us manually do this iostream
		 cmd = getl(); 
		//cmd = "put a.txt hello";
		//fgets(s,2048,stdin);
		//cmd = s;
		if (cmd=="logout") return 2;
		if (cmd=="exit") {
			if (elevstack) {
				elevstack--;
				env_user = curlogin.account_name;
				continue;
			}
			else return 2;
		}
		if (cmd=="halt"||cmd=="reboot") {
			if (curlogin.account_premission > 0 || elevstack != 0) {
				if (cmd=="halt") return 0;
				if (cmd=="reboot") return 3;
			} else {
				printf("%s: Permission denied\n",cmd.c_str());
			}
			continue;
		}
		if (cmd=="elev") {
			if (curlogin.account_premission <= 0 && elevstack == 0) {
				do {
					string login_pwd;
					printf("Input password of %s: ",getAdminInfo().account_name.c_str());
					login_pwd = pwd_input();
					printf("\n");
					if (login_pwd == getAdminInfo().account_password) break;
					printf("Incorrect password. Try again.\n");
				} while (1);
				elevstack = 1;
				env_user = getAdminInfo().account_name;
			}
			continue;
		}
		errval = call_cmd(f,split_arg(cmd,true));
	}
}

int main() {
	// debugging management
	put(3,split_arg("put a.txt hi",true));
	// -end-
	initz: initalize();
	logz: login();
	switch (shell()) {
		case 0:
			return 0;
			break;
		case 1: case 3:
			goto initz;
			break;
		case 2:
			goto logz;
			break;
	}
	return 0;
} 
