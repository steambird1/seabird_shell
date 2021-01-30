#include "filesystem.hpp"
#include "accounts.hpp"
#include "shellexec.hpp"
#include "cmdtfunc.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <time.h>
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

int getFileSize(const char* fname)
{
	FILE *fp;
	fp = fopen(fname,"r");
    fseek(fp, 0L, SEEK_END);
    int size = ftell(fp);
    fclose(fp);
    return size;
}

#define helpls "Usage: ls [-F|-f]\n\
ls without parameter will list folders and files under current folder.\n\
-F argument will only list folders, -f argument will only list files."

#define helpput "Usage: put [-o] <file> <data>\n\
Put a line of data to a file.\n\
You can override file instead of append file with '-o' switch."

#define helpfo "Usage: mv [-o] <source> <dest>\n\
cp [-o] <source> <dest>\n\
\n\
Move or copy file to another file.\n\
You can override existing file with '-o' switch.\n\
If you does not open this switch, program will be stopped if target exists."

#define helplong "Long commands\n\
Long commands allow you to do operate using full path.\n\
\n\
Long commands are:\n\
write => put\n\
type => cat\n\
move => mv\n\
mkdir => md\n\
copy => cp\n\
del => rm\n\
edit => notepad"

#define helpsvt "Usage: svt (import|output) <guest filename> <host filename>\n\
Import mode to import a file from host to guest.\n\
Output mode to output a file from guest to host."

#define helpmsg "Welcome to seabird Galactic shell\n\
\n\
(type 'help longfor' for long-format commands.)\n\
Commands available are:\n\
help [title]              Show some help message.\n\
echo <msg>                Show <msg>.\n\
clear                     Clear screen.\n\
errval                    Get the previous command's return value.\n\
ls [-F|-f]                List folders and files. For more informations type 'help ls'.\n\
put [-o] <file> <data>    Output a line of <data> to <file>. For more informations type 'help put'.\n\
cat <file>                Output file content.\n\
mv [-o] <source> <dest>   Move file. for more information of this command, type 'help fileoperate'.\n\
cp [-o] <source> <dest>   Copy file. for more information of this command, type 'help fileoperate'.\n\
rm <file>                 Remove file.\n\
md <name>                 Create a new directory.\n\
edit <file>               Edit a file using windows notepad.\n\
cd (<dir>|..)             Switch 'current directory'. '..' for turn to previous folder.\n\
color <color string>      Change the output text's color.\n\
whoami                    Output the logged user name.\n\
logout                    Logout from a login.\n\
exit                      Logout from a login, or exit from a elevated shell.\n\
elev                      Load elevated shell to run administrator commands in non-administrator account.\n\
halt                      Turn off this shell.\n\
reboot                    Reload this shell.\n\
rand [min] [max]          Pick a random number.\n\
svt ...                   Share file between this shell and host OS. for more information type 'help svt'."

int help(int argc, vector<string> argv) {
	if (argc==1) printf("%s\n",helpmsg);
	if (argc==2) {
		if (argv[1]=="ls") printf("%s\n",helpls);
		else if (argv[1]=="put") printf("%s\n",helpput);
		else if (argv[1]=="fileoperate") printf("%s\n",helpfo);
		else if (argv[1]=="longfor") printf("%s\n",helplong);
		else if (argv[1]=="svt") printf("%s\n",helpsvt);
		else {
			printf("Invaild help title\n");
			return 1;
		}
	}
	return 0;
}

/*
The commands will be add:

echo	Echo a message. (completed) 
clear	Clear screen. (completed)
errval	Ouput previous return value. (completed)
ls		List folders and files. (-F for only folders, -f for only files.) (completed)

put		Output a line to file. (-o for override) (completed)
cat		View file. (completed)
mv		Move file. (-o for override) (completed)
md		Make directory. (completed)
cp		Copy file. (-o for override) (completed) * Unlucky, 'cp', 'mv' and 'rm' only supports execute in same folder.
rm		Remove file. (completed)
edit	Call notepad to edit file saved in it. (I can only do this.) 
------------------------------------
Full path version:
------------------------------------
write => put
type => cat
move => mv
mkdir => md
copy => cp
del => rm
edit => notepad
-END-
rd		Remove directory. ! for some reason I can't complete it
ren		Remove current directory. ! for some reason I can't complete it, neither...
(What a great folder!)

cd		Switch 'current directory'. (completed)
color	Change command line output color.
whoami	User information. (completed)
logout	Logout. (*)
exit	Logout or logout elevated shell. (*)
elev	Make elevated permission. (*)
halt	Exit virtual shell. (*)
reboot	Reboot virtual shell. (*)
rand	Pick a random number. (you should specify lowercase and/or uppercase, or a really random if not specified.)
*/

/*(continue)
svt		Virtual shell file tools (share files between host and app).

help	Show help message.

The shell prompt like:

[username]@[host name] [current directory]$ [inputting]

(*) means builtin command.
You can add your command, too.
*/

int _ran(void) {
	static int seed = time(NULL);
	srand(seed);
	seed = rand();
	return seed;
}

int _rhi(int hi) {
	return _ran()%hi;
}

int _rlh(int low,int hi) {
	return _ran()%(hi-low)+low;
}

int random(int argc, vector<string> argv) {
	switch (argc) {
		case 1:
			cout << _ran() << endl;
			break;
		case 2:
			cout << _rhi(atoi(argv[1].c_str())) << endl;
			break;
		case 3:
			cout << _rlh(atoi(argv[1].c_str()),atoi(argv[2].c_str())) << endl;
			break;
	}
	return 0;
}

string _getTemp(void) {
	string s;
	s = getenv("temp");
	s = s + "\\sgshell_tempedit";
	return s;
}

int editor(int argc, vector<string> argv) {
	if (argc < 2) {
		cout << "Required parameter missing" << endl;
		return 1;
	}
	if (!isFileExistsA(root,cdir,argv[1])) {
		cout << "Specified file not exist" << endl;
		return 1;
	}
	FILE *f;
	f = fopen(_getTemp().c_str(),"w+");
	string buf = readFileA(root,cdir,argv[1]);
	int bytes = 0;
	cout << "Byte proceed:           0";
	for (int i = 0; i < buf.length(); i++) {
		printf("\b\b\b\b\b\b\b\b\b\b%10d",++bytes);
		fputc(buf[i],f);
	}
	cout<<endl;
	fclose(f);
	string scmd = "notepad ";
	scmd = scmd + _getTemp();
	system(scmd.c_str());
	// after notepad, push changes:
	buf = "";
	bytes = 0;
	cout << "Modified file (" << getFileSize(_getTemp().c_str()) << " Bytes )" << endl;
	f = fopen(_getTemp().c_str(),"r");
	cout << "Byte proceed:           0";
	while (!feof(f)) {
		printf("\b\b\b\b\b\b\b\b\b\b%10d",++bytes);
		buf = buf + char(fgetc(f));
	}
	fclose(f);
	modifyFileA(root,cdir,argv[1],buf);
	cout<<endl;
	return 0;
}

int notepad(int argc, vector<string> argv) {
	string cdr = getFirst(argv[1]), arg1 = getLast(argv[1]);
	if (argc < 2) {
		cout << "Required parameter missing" << endl;
		return 1;
	}
	if (!isFileExistsA(root,cdr,arg1)) {
		cout << "Specified file not exist" << endl;
		return 1;
	}
	FILE *f;
	f = fopen(_getTemp().c_str(),"w+");
	string buf = readFileA(root,cdir,argv[1]);
	int bytes = 0;
	cout << "Byte proceed:           0";
	for (int i = 0; i < buf.length(); i++) {
		printf("\b\b\b\b\b\b\b\b\b\b%10d",++bytes);
		fputc(buf[i],f);
	}
	cout<<endl;
	fclose(f);
	string scmd = "notepad ";
	scmd = scmd + _getTemp();
	system(scmd.c_str());
	// after notepad, push changes:
	buf = "";
	cout << "Modified file (" << getFileSize(_getTemp().c_str()) << " Bytes )" << endl;
	f = fopen(_getTemp().c_str(),"r");
	bytes = 0;
	cout << "Byte proceed:           0";
	while (!feof(f)) {
		printf("\b\b\b\b\b\b\b\b\b\b%10d",++bytes);
		buf = buf + char(fgetc(f));
	}
	fclose(f);
	modifyFileA(root,cdr,arg1,buf);
	cout<<endl;
	return 0;
}

int svt(int argc, vector<string> argv) {
	// svt import [pos] [local filename]
	// svt output [pos] [local filename]
	// Use quotes for long path.
	if (argc < 4) {
		cout << "Required parameter missing" << endl;
		return 1;
	}
	if (argv[1]=="import") {
		if (isFileExistsA(root,cdir,argv[2])) {
			cout << "Specified file already exists" << endl;
			return 3;
		}
		string buf = "";
		FILE *f;
//		cout << "Modified file (" << getFileSize(_getTemp().c_str()) << " Bytes" << endl;
		cout << "Reading from: " << argv[3] << " ( " << getFileSize(argv[3].c_str()) << " Bytes)" <<endl << "Byte proceed:           0";
		f = fopen(argv[3].c_str(),"r");
		int bytes = 0;
		while (!feof(f)) {
			printf("\b\b\b\b\b\b\b\b\b\b%10d",++bytes);
			buf = buf + char(fgetc(f));
		}
		fclose(f);
		if (buf=="") {
			cout << "Specified file is empty" << endl;
			return 4;
		} 
		createFileA(root,cdir,argv[2],buf);
		cout<<endl;
		return 0;
	} else if (argv[1]=="output") {
		if (!isFileExistsA(root,cdir,argv[2])) {
			cout << "Specified file does not exist" << endl;
		}
		FILE *f;
		int bytes = 0;
		string buf = readFileA(root,cdir,argv[2]);
		//cout << "Writing to: " << argv[3] << "( " << buf.length() << " Bytes)"¡¡<< endl << "Byte proceed:           0";
		cout << "Writing to: " << argv[3] << " ( " << buf.length() << " Bytes)" << endl << "Byte proceed:           0";
		f = fopen(argv[3].c_str(),"w+");
		for (int i = 0; i < buf.length(); i++) {
			printf("\b\b\b\b\b\b\b\b\b\b%10d",++bytes);
			fputc(buf[i],f);
		}
		fclose(f);
		cout<<endl;
		return 0;
	} else {
		cout << "Invaild operation" << endl;
		return 2;
	}
}

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
	dat = dat + "\n";
	_proceedFile(resolve(cdir,root),fn,dat);
	return !isFileExistsA(root,"/",fn);
}

int write(int argc, vector<string> argv) {
	bool override = false;
	if (argc >= 2 && argv[1] == "-o") override = true;
	string dat = "", fn = getLast(argv[int(override)+1]),cdr = getFirst(argv[int(override)+1]);
	if ((!override) && isFileExistsA(root,cdr,fn)) dat = readFileA(root,cdr,fn);
	if ((override&&argc>=4)||(argc>=3)) for (vector<string>::iterator i = argv.begin()+int(override)+2; i != argv.end(); i++) dat = dat + (*i) + " ";
	dat = dat + "\n";
	_proceedFile(resolve(cdr,root),fn,dat);
	return !isFileExistsA(root,"/",fn);
}

int type(int argc, vector<string> argv) {
	if (argc < 2) return 1;
	string cdr = getFirst(argv[1]), fn = getLast(argv[1]);
	if (!isFileExistsA(root,cdr,fn)) {
		cout << "Specified file does not exist" << endl;
		return 2;
	}
	cout << readFileA(root,cdr,fn) << endl;
	return 0;
}

int cat(int argc, vector<string> argv) {
	if (argc < 2) return 1;
	string fn = argv[1];
	if (!isFileExistsA(root,cdir,fn)) {
		cout << "Specified file does not exist" << endl;
		return 2;
	}
	cout << readFileA(root,cdir,fn) << endl;
	return 0;
}

int copy(int argc, vector<string> argv) {
	if (argc < 3) {
		cout << "Required parameter missing" << endl;
		return 1;
	}
	bool override = false;
	if (argc >= 2 && argv[1] == "-o") override = true;
	string sc = argv[1+int(override)], dc = argv[2+int(override)];
	string cdr1 = getFirst(sc), cdr2 = getFirst(dc), source = getLast(sc), dest = getLast(dc);
	cout << cdr1 << "/" << source << " ; " << cdr2 << "/" << dest << endl;//testing
	if (!isFileExistsA(root,cdr1,source)) {
		cout << "Source file does not exist" << endl;
		return 2;
	}
	if (isFileExistsA(root,cdr2,dest)&&(!override)) {
		cout << "Target already exists (for override using '-o')" << endl;
		return 3;
	} 
	copyFileA(root,cdr1,source,cdr2,dest);
	return 0;
}

int cp(int argc, vector<string> argv) {
	if (argc < 3) {
		cout << "Required parameter missing" << endl;
		return 1;
	}
	bool override = false;
	if (argc >= 2 && argv[1] == "-o") override = true;
	if (!isFileExistsA(root,cdir,argv[1+int(override)])) {
		cout << "Source file does not exist" << endl;
		return 2;
	}
	if (isFileExistsA(root,cdir,argv[2+int(override)])&&(!override)) {
		cout << "Target already exists (for override using '-o')" << endl;
		return 3;
	} 
	copyFileA(root,cdir,argv[1+int(override)],cdir,argv[2+int(override)]);
	return 0;
}

int move(int argc, vector<string> argv) {
	if (argc < 3) {
		cout << "Required parameter missing" << endl;
		return 1;
	}
	bool override = false;
	if (argc >= 2 && argv[1] == "-o") override = true;
	string sc = argv[1+int(override)], dc = argv[2+int(override)];
	string cdr1 = getFirst(sc), cdr2 = getFirst(dc), source = getLast(sc), dest = getLast(dc);
	if (!isFileExistsA(root,cdr1,source)) {
		cout << "Source file does not exist" << endl;
		return 2;
	}
	if (isFileExistsA(root,cdr2,dest)&&(!override)) {
		cout << "Target already exists (for override using '-o')" << endl;
		return 3;
	} 
	moveFileA(root,cdr1,source,cdr2,dest);
	return 0;
}

int mv(int argc, vector<string> argv) {
	if (argc < 3) {
		cout << "Required parameter missing" << endl;
		return 1;
	}
	bool override = false;
	if (argc >= 2 && argv[1] == "-o") override = true;
	if (!isFileExistsA(root,cdir,argv[1+int(override)])) {
		cout << "Source file does not exist" << endl;
		return 2;
	}
	if (isFileExistsA(root,cdir,argv[2+int(override)])&&(!override)) {
		cout << "Target already exists (for override using '-o')" << endl;
		return 3;
	} 
	moveFileA(root,cdir,argv[1+int(override)],cdir,argv[2+int(override)]);
	return 0;
}

int del(int argc, vector<string> argv) {
	if (argc < 2) {
		cout << "Required parameter missing" << endl;
		return 1;
	}
	string cdr = getFirst(argv[1]), fn = getLast(argv[1]);
	if (!isFileExistsA(root,cdr,fn)) {
		cout << "Source file does not exist" << endl;
		return 2;
	}
	rmFileA(root,cdr,fn);
}

int rm(int argc, vector<string> argv) {
	if (argc < 2) {
		cout << "Required parameter missing" << endl;
		return 1;
	}
	if (!isFileExistsA(root,cdir,argv[1])) {
		cout << "Source file does not exist" << endl;
		return 2;
	}
	rmFileA(root,cdir,argv[1]);
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
			v = listFileA(root,dr,2);
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

int md(int argc, vector<string> argv) {
	if (argc < 2) {
		cout << "Required parameter missing" << endl;
		return 2;
	}
	if (isSubdirExistsA(root,cdir,argv[1])) {
		cout << "Directory already exists" << endl;
		return 1;
	}
	createFolderA(root,cdir,argv[1]);
	return 0;
}

int mkdir(int argc, vector<string> argv) {
	if (argc < 2) {
		cout << "Required parameter missing" << endl;
		return 2;
	}
	string cdr = getFirst(argv[1]), fn = getLast(argv[1]);
	if (isSubdirExistsA(root,cdr,fn)) {
		cout << "Directory already exists" << endl;
		return 1;
	}
	createFolderA(root,cdr,fn);
	return 0;
}

int cd(int argc, vector<string> argv) {
	if (argc < 2) {
		cout << "Required parameter missing" << endl;
		return 2;
	}
	if (argv[1]=="..") {
		// turn to previous
		cdir=getFirst(cdir);
		return 0;
	}
	if (!isSubdirExistsA(root,cdir,argv[1])) {
		cout << "Directory not exist" << endl;
		return 1;
	}
	string mask = "";
	if (cdir!="/") mask="/"; 
	if (cdir!="/") mask = "/";
	cdir = cdir + mask + argv[1];
	return 0;
}

int colors(int argc, vector<string> argv) {
	if (argc < 2) {
		cout << "Required parameter missing" << endl;
		return 2;
	}
	setColor(argv[1]);
	return 0;
}

int rd(int argc, vector<string> argv) {
	if (cdir=="/") {
		cout << "Permission denied" << endl;
		return 1;
	}
	rmDirA(root,cdir);
	cdir=getFirst(cdir);
	return 0;
}

int ren(int argc, vector<string> argv) {
	if (cdir=="/") {
		cout << "Permission denied" << endl;
		return 1;
	}
	if (argc < 2) {
		cout << "Required parameter missing" << endl;
		return 2;
	}
	renameFolderA(root,cdir,argv[1]);
	cdir=getFirst(cdir);
	string mask = "";
	if (cdir!="/") mask="/";
	cdir=cdir+mask+argv[1];
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
	f["write"]=write;
	f["cat"]=cat; 
	f["type"]=type;
	f["cp"]=cp;
	f["copy"]=copy;
	f["mv"]=mv;
	f["move"]=move;
	f["rm"]=rm;
	f["del"]=del;
	f["md"]=md;
	f["mkdir"]=mkdir;
	f["cd"]=cd;
	f["color"]=colors;
	f["svt"]=svt;
	f["edit"]=editor;
	f["notepad"]=notepad;
	f["rand"]=random;
	f["help"]=help;
	f["rd"]=rd;
	f["ren"]=ren;
}

#define KERNEL_VER "1.1.0.30"
#define SYS_ARCH "unknown architecture"

void login(void) {
	// Login page
	string login_name, login_pwd;
	clear();
	printf("Seabird Galactic\nKernel %s on %s\n\n",KERNEL_VER,SYS_ARCH);
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
		if (errval == -1) printf("Bad command\n");
	}
}

int main() {
	// debugging management
	//put(3,split_arg("put a.txt hi",true));
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
