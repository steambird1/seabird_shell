#include "filesystem.hpp"
#include "accounts.hpp"
#include "shellexec.hpp"
#include "cmdtfunc.h"
#include "scapack.hpp"
//#include "sccio.hpp" // sccio is not used for normal shell, at least currently.
#include "scpart.hpp" 
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <time.h>

// package!
#include "app_swriter.hpp"
// end
using namespace std; 

// Updated. recompile required ... 
// declare,
int _call_pare(string);
int _call_chroot(string);

disk d;
int d_lastname = 1;

appacks r;
funcall f;
map<string,string> extcall;//extern calls
acclist ac;
bool logged = false;
account curlogin;
int elevstack = 0,errval = 0;

string cdir = "/", env_user = "";
fdirnode *root = new fdirnode;

vector<string> empty_argv;

int getFileSize(const char* fname)
{
	FILE *fp;
	fp = fopen(fname,"r");
    fseek(fp, 0L, SEEK_END);
    int size = ftell(fp);
    fclose(fp);
    return size;
}

string getRealFirst(string path) {
	if (path=="/") return "/";
	if (path[0]=='/') return getFirst(path);
	string mask = "";
	if (cdir!="/") mask="/";
	//cout << cdir + mask + path << " " << mask << endl; 
	//cout << getFirst(cdir + mask + path) << endl; 
	return getFirst(cdir + mask + path);
}

inline string getRealLast(string path) {
	return getLast(path);
}

string getRealDir(string dir) {
	if (dir=="/") return "/";
	if (dir[0]=='/') return dir;
	string mask = "/";
	if (cdir=="/") mask="";
	return cdir + mask + dir;
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
The short commands and long commands has been merged currently.\n\
Following commands are same.\n\
\n\
Same commands are:\n\
write => put\n\
type => cat\n\
move => mv\n\
mkdir => md\n\
copy => cp\n\
del => rm\n\
edit => notepad"

#define helpsvt "Usage: svt <function> <guest filename> <host filename>\n\
Functions are:\n\
\n\
import         To import a file from host to guest.\n\
output         To output a file from guest to host.\n\
declare        To declare a external command (call a command on your host).\n\
export         To output files in a folder from guest to host.\n\
"
/*
		q = quit
		o = open <filename>
		x = close
		a = add <line-pos> <data>
		p = append <data>
		e = edit <line-pos> <data>
		d = delete <line-pos>
		c = copy <line-pos> <line-pos>
		m = move <line-pos> <line-pos>
		s = save
		v = view [line-number]
		r = search <data> 
		l = replace <origin> <string>
		*/
#define helpsedit "Usage: sedit [filename]\n\
'filename' specify the filename to open.\n\
\n\
Editor commands are:\n\
q = quit\n\
o = open <filename>\n\
x = close\n\
a = add <line-pos> <data>\n\
p = append <data>\n\
e = edit <line-pos> <data>\n\
d = delete <line-pos>\n\
c = copy <line-pos> <line-pos>\n\
m = move <line-pos> <line-pos>\n\
s = save\n\
v = view [line-number]\n\
r = search <data> \n\
l = replace <origin> <string>"

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
ren [directory] <name>    Rename a directory.\n\
rd [directory]            Delete a directory.\n\
edit <file>               Edit a file using windows notepad.\n\
sedit [file]              Edit a file using seabird notepad. For more information of command, type 'help sedit'.\n\
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



string readFromArg(vector<string> arg,int pos) {
	string buf = "";
	for (vector<string>::iterator i = arg.begin()+pos; i != arg.end(); i++) buf = buf + (*i) + " ";
	return buf;
}

string subreplace(string resource_str, string sub_str, string new_str)
{
    string::size_type pos = 0;
    while((pos = resource_str.find(sub_str)) != string::npos)   //替换所有指定子串
    {
        resource_str.replace(pos, sub_str.length(), new_str);
    }
    return resource_str;
}

int sword(int argc, vector<string> argv) {
	if (!r.appalist["seabird-galactic-wordpad"].install_stat) {
		cout << "Bad command" << endl << endl << "You can install it by typing:" << endl << "apack seabird-galactic-wordpad" << endl << endl;//require re-compile
		return 3;
	}
	string pathopen = "", fnopen = "";
	if (argc < 2) {
		cout << "Required parameter missing" << endl;
		return 1;
	}
	pathopen = getRealFirst(argv[1]);
	fnopen = getRealLast(argv[1]);
	if (!isFileExistsA(root,pathopen,fnopen)) {
		//cout << "Specified file does not exist" << endl;
		//return 2;
		createFileA(root,pathopen,fnopen,"");
	}
	string fs = swAppMain(readFileA(root,pathopen,fnopen));
	if (fs != "") modifyFileA(root,pathopen,fnopen,fs);
	return 0;
}

int seditor(int argc, vector<string> argv) {
	string pathopen = "", fnopen = "";
	vector<string> buf;
	vector<string> empty_vector;
	if (argc >= 2) {
		pathopen = getRealFirst(argv[1]);
		fnopen = getRealLast(argv[1]);
		if (!isFileExistsA(root,pathopen,fnopen)) {
			cout << "Specified file does not exist" << endl;
			pathopen = "";
			fnopen = "";
		} else {
			buf = spiltLines(readFileA(root,pathopen,fnopen));
		}
	}
	string cm;
	vector<string> argz;
	do {
		cout << "> ";
		cm = getl();
		argz = split_arg(cm,true);
		string fitem = argz[0];
		
		// Line ID starts with 0!!!!
		string pu = "";
		bool flag = false;
		switch (fitem[0]) {
			case 'q':
				return 0;
			case 'o':
				{
					if (argz.size() >= 2) {
		pathopen = getRealFirst(argz[1]);
		fnopen = getRealLast(argz[1]);
		if (!isFileExistsA(root,pathopen,fnopen)) {
			cout << "Specified file does not exist" << endl;
			pathopen = "";
			fnopen = "";
		} else {
			buf = spiltLines(readFileA(root,pathopen,fnopen));
		}
	} else {
		cout << "Required parameter missing" << endl;
	}
				}
				break;
			case 'x':
				{
					pathopen = "";
				fnopen = "";
				buf = empty_vector;
				}
				break;
			case 'a':
				if (argz.size() < 3) {
					cout << "Required parameter missing" << endl;
					break;
				}
				// insert to buf
				buf.insert(buf.begin()+atoi(argz[1].c_str()),readFromArg(argz,2));
				break;
			case 'p':
				if (argz.size() < 2) {
					cout << "Required parameter missing" << endl;
					break;
				}
				buf.push_back(readFromArg(argz,1));
				break;
			case 's':
				pu = "";
				for (vector<string>::iterator i = buf.begin(); i < buf.end(); i++) {
					pu = pu + (*i) + "\n";
				}
				modifyFileA(root,pathopen,fnopen,pu);
				break;
			case 'v':
					if (argz.size() < 2) {
						int a = 0;
					for (vector<string>::iterator i = buf.begin(); i < buf.end(); i++) {
						cout << a++ << " " << (*i) << endl;
					}
				} else {
					cout << buf[atoi(argz[1].c_str())] << endl;
				}
				break;
			case 'e':
				{
					if (argz.size() < 3) {
					cout << "Required parameter missing" << endl;
					break;
				}
				buf[atoi(argz[1].c_str())] = readFromArg(argz,2);
				break;
				}
			case 'd':
				if (argz.size() < 2) {
					cout << "Required parameter missing" << endl;
					break;
				}
				buf.erase(buf.begin()+atoi(argz[1].c_str()));
				break;
			case 'c':
				if (argz.size() < 3) {
					cout << "Required parameter missing" << endl;
					break;
				}
				buf.insert(buf.begin()+atoi(argz[2].c_str()),buf[atoi(argz[1].c_str())]);
				break;
			case 'm':
				if (argz.size() < 3) {
					cout << "Required parameter missing" << endl;
					break;
				}
				buf.insert(buf.begin()+atoi(argz[2].c_str()),buf[atoi(argz[1].c_str())]);
				buf.erase(buf.begin()+atoi(argz[1].c_str()));
				break;
			case 'r':
				if (argz.size() < 2) {
					cout << "Required parameter missing" << endl;
					break;
				}
				flag = false;
				for (int i = 0; i < buf.size(); i++) {
						int res = buf[i].find(argz[1]);
						if (res != string::npos) {
							cout << "Found searching string at line " << i << ", col " << res << endl;
							flag = true;
						}
					}
				break;
			case 'l':
				if (argz.size() < 3) {
					cout << "Required parameter missing" << endl;
					break;
				}
				for (int i = 0; i < buf.size(); i++) {
						//buf[i] = buf[i].replace(argz[1],argz[2]);
						buf[i] = subreplace(buf[i],argz[1],argz[2]);
					}
				break;
		}
	} while (1);
}

void execute_command(string cmd) {
	vector<string> v;
		v = split_arg(cmd,true);
		errval = call_cmd(f,v);
		if (errval == -1) {
			if (extcall.count(v[0])) {
				string comd = extcall[v[0]];
				for (int i = 1; i < v.size(); i++) comd = comd + " " + v[i];
			//	cout << v[0] << " " << extcall[v[0]] << " " << comd << endl;
				errval = system(comd.c_str());
			} else printf("Bad command\n");
		}
}

int runscript(int argc, vector<string> argv) {
	if (argc < 2) {
		cout << "Required parameter missing" << endl;
		return 2;
	}
	string cdr = getRealFirst(argv[1]), fn = getRealLast(argv[1]);
	if (!isFileExistsA(root,cdr,fn)) {
		cout << "File not exist" << endl;
		return 1;
	}
	vector<string> cs;
	cs = spiltLines(readFileA(root,cdr,fn));
	for (vector<string>::iterator i = cs.begin(); i != cs.end(); i++) {
		execute_command((*i));
	}
	return 0;
} 

int help(int argc, vector<string> argv) {
	if (argc==1) printf("%s\n",helpmsg);
	if (argc==2) {
		if (argv[1]=="ls") printf("%s\n",helpls);
		else if (argv[1]=="put") printf("%s\n",helpput);
		else if (argv[1]=="fileoperate") printf("%s\n",helpfo);
		else if (argv[1]=="longfor") printf("%s\n",helplong);
		else if (argv[1]=="svt") printf("%s\n",helpsvt);
		else if (argv[1]=="sedit") printf("%s\n",helpsedit);
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
int notepad(int argc, vector<string> argv) {
	string cdr = getRealFirst(argv[1]), arg1 = getRealLast(argv[1]);
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
	string buf = readFileA(root,cdr,arg1);
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

FILE *svt_output;

void _preserve_list(string rootpath) {
	string loadfile;
	vector<string> ls;
	ls = listFileA(root,rootpath,1);
	for (vector<string>::iterator i = ls.begin(); i != ls.end(); i++) {
		string dirnamez = (*i).substr(1,(*i).length()-2);
		if (dirnamez == "." || dirnamez == "..") continue;
		string mask = "/";
		if (rootpath=="/") mask="";
		cout << "Reading directory: " << rootpath + mask + dirnamez << endl;
		loadfile = "1 " + rootpath + " " + dirnamez + "\n"; // say goodbye to mask !!!
		fprintf(svt_output,"%s",loadfile.c_str());
		_preserve_list(rootpath + mask + dirnamez);
	}
	ls = listFileA(root,rootpath,2);
	for (vector<string>::iterator i = ls.begin(); i != ls.end(); i++) {
		string mask = "/";
		if (rootpath=="/") mask="";
		string buf = readFileA(root,rootpath,(*i));
		cout << "Reading file: " << rootpath + mask + (*i) << endl;
		char c[10];
		itoa(buf.length(),c,10);
		loadfile = "0 " + rootpath + " " + (*i) + " " + c + "\n" + buf + "\n"; // say goodbye to mask!
		fprintf(svt_output,"%s",loadfile.c_str());
	}
}

int svt(int argc, vector<string> argv) {
	cout << "SVT Tools V2.0.0" << endl; 
	// svt import [pos] [local filename]
	// svt output [pos] [local filename]
	// svt declare [command] [local command]
	// svt export [local directory]
	
	// to add:
	// svt save [local disk file] (save current root)
	// svt load [local disk file] (to a new partition)
	// Use quotes for long path.
	if (argc < 3) {
		cout << "Required parameter missing" << endl;
		return 1;
	}
	if (argv[1]=="import") {
		if (argc < 4) {
		cout << "Required parameter missing" << endl;
		return 1;
	}
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
		cout<<"Operation completed successfully."<<endl;
		return 0;
	} else if (argv[1]=="output") {
		if (argc < 4) {
		cout << "Required parameter missing" << endl;
		return 1;
	}
		if (!isFileExistsA(root,cdir,argv[2])) {
			cout << "Specified file does not exist" << endl;
		}
		FILE *f;
		int bytes = 0;
		string buf = readFileA(root,cdir,argv[2]);
		//cout << "Writing to: " << argv[3] << "( " << buf.length() << " Bytes)"　<< endl << "Byte proceed:           0";
		cout << "Writing to: " << argv[3] << " ( " << buf.length() << " Bytes)" << endl << "Byte proceed:           0";
		f = fopen(argv[3].c_str(),"w+");
		for (int i = 0; i < buf.length(); i++) {
			printf("\b\b\b\b\b\b\b\b\b\b%10d",++bytes);
			fputc(buf[i],f);
		}
		fclose(f);
		cout<<endl;
		cout<<"Operation completed successfully."<<endl;
		return 0;
	} else if (argv[1]=="declare") {
		if (argc < 4) {
		cout << "Required parameter missing" << endl;
		return 1;
	}
		extcall[argv[2]]=argv[3];
		cout<<"Operation completed successfully."<<endl;
		return 0;
	} else if (argv[1]=="export") {
		if (argc < 3) {
		cout << "Required parameter missing" << endl;
		return 1;
	}
		vector<string> v;
		v = listFileA(root,cdir,2);
		if (v.size()==0) {
			cout << "No file for output" << endl;
			return 1;
		}
		FILE *f;
		for (int i = 0; i < v.size(); i++) {
			string buf = readFileA(root,cdir,v[i]), fo = argv[2] + "\\" + v[i];
			cout << "Outputting " << v[i] << " ( " << buf.length() << " Bytes, " << i + 1 << " of " << v.size() << " ) Byte proceed:           0";
			int bytes = 0;
			f = fopen(fo.c_str(),"w+");
			for (int j = 0; j < buf.length(); j++) {
				printf("\b\b\b\b\b\b\b\b\b\b%10d",++bytes);
				fputc(buf[j],f);
			}
			fclose(f);
			cout << endl;
		} 
		cout<<"Operation completed successfully."<<endl;
		return 0;
	} else if (argv[1]=="save") {
		cout << "Reading document tree ..." << endl;
		svt_output = fopen(argv[2].c_str(),"w+");
		_preserve_list("/");
		fprintf(svt_output,"-1\n");
		fclose(svt_output);
	} else if (argv[1]=="load") {
		_call_pare("new");
		char c[5];
		itoa(d_lastname,c,10);
		string cd = c;
		_call_chroot(cd);
		FILE *svt_input;
		svt_input = fopen(argv[2].c_str(),"r+");
		int mode,fsize;
		char filepath[2048],filename[512];
		string fpath,fname,buf;
		while (fscanf(svt_input,"%d",&mode)!=EOF) {
			if (mode==-1) break;
			switch (mode) {
				case 0:
					fscanf(svt_input,"%s%s%d",filepath,filename,&fsize);
					fgetc(svt_input);//feed '\n'
					buf = "";
					for (int i = 0; i < fsize; i++) {
						buf = buf + char(fgetc(svt_input));
					}
					fpath = filepath; fname = filename;
					cout << "Writing file " << fname << " to " << fpath << endl;
					createFileA(root,fpath,fname,buf);
					break;
				case 1:
					fscanf(svt_input,"%s%s",filepath,filename);
					fpath = filepath; fname = filename;
					cout << "Creating folder " << fpath << endl;
					createFolderA(root,fpath,fname);
					break;
			}
		}
		cout << "Operation completed successfully." << endl;
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
int write(int argc, vector<string> argv) {
	bool override = false;
	if (argc >= 2 && argv[1] == "-o") override = true;
	string dat = "", fn = getRealLast(argv[int(override)+1]),cdr = getRealFirst(argv[int(override)+1]);
	if ((!override) && isFileExistsA(root,cdr,fn)) dat = readFileA(root,cdr,fn);
	if ((override&&argc>=4)||(argc>=3)) for (vector<string>::iterator i = argv.begin()+int(override)+2; i != argv.end(); i++) dat = dat + (*i) + " ";
	dat = dat + "\n";
	_proceedFile(resolve(cdr,root),fn,dat);
	return !isFileExistsA(root,"/",fn);
}

int type(int argc, vector<string> argv) {
	if (argc < 2) return 1;
	string cdr = getRealFirst(argv[1]), fn = getRealLast(argv[1]);
	if (!isFileExistsA(root,cdr,fn)) {
		cout << "Specified file does not exist" << endl;
		return 2;
	}
	cout << readFileA(root,cdr,fn) << endl;
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
	string cdr1 = getRealFirst(sc), cdr2 = getRealFirst(dc), source = getRealLast(sc), dest = getRealLast(dc);
	//cout << cdr1 << "/" << source << " ; " << cdr2 << "/" << dest << endl;//testing
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
int move(int argc, vector<string> argv) {
	if (argc < 3) {
		cout << "Required parameter missing" << endl;
		return 1;
	}
	bool override = false;
	if (argc >= 2 && argv[1] == "-o") override = true;
	string sc = argv[1+int(override)], dc = argv[2+int(override)];
	string cdr1 = getRealFirst(sc), cdr2 = getRealFirst(dc), source = getRealLast(sc), dest = getRealLast(dc);
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
int del(int argc, vector<string> argv) {
	if (argc < 2) {
		cout << "Required parameter missing" << endl;
		return 1;
	}
	string cdr = getRealFirst(argv[1]), fn = getRealLast(argv[1]);
	if (!isFileExistsA(root,cdr,fn)) {
		cout << "Source file does not exist" << endl;
		return 2;
	}
	rmFileA(root,cdr,fn);
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
		if (argc >= 3) dr = getRealDir(argv[2]);
		if (argv[1]=="-F") {
			v = listFileA(root,dr,1);
			flag = true;
		} else if (argv[1]=="-f") {
			v = listFileA(root,dr,2);
			flag = true;
		} else {
			dr = getRealDir(argv[1]);
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

int mkdir(int argc, vector<string> argv) {
	if (argc < 2) {
		cout << "Required parameter missing" << endl;
		return 2;
	}
	string cdr = getRealFirst(argv[1]), fn = getRealLast(argv[1]);
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
	if (argv[1]==".") {
		return 0;
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
	
	string cdr = cdir;
	if (argc == 2) {
		cdr = getRealDir(argv[1]);
		if (cdr=="/") {
			cout << "Permission denied" << endl;
			return 1;
		}
		if (!isSubdirExistsA(root,getFirst(cdr),getLast(cdr))) {
			cout << "Specified directory does not exist" << endl;
			return 2;
		}
	} else {
		if (cdir=="/") {
			cout << "Permission denied" << endl;
			return 1;
		}
	} 
	rmDirA(root,cdr);
	if (cdr == cdir) cdir=getFirst(cdir);
	else cdir=getFirst(cdr); 
	return 0;
}

int ren(int argc, vector<string> argv) {
	if (argc < 2) {
		cout << "Required parameter missing" << endl;
		return 2;
	}
	string cdr = cdir,nam = argv[1];
	if (argc == 3) {
		cdr = getRealDir(argv[1]); 
		nam = argv[2];
		if (cdr=="/") {
			cout << "Permission denied" << endl;
			return 1;
		} 
		if (!isSubdirExistsA(root,getFirst(cdr),getLast(cdr))) {
			cout << "Specified directory does not exist" << endl;
			return 2;
		}
	} else {
		if (cdir=="/") {
		cout << "Permission denied" << endl;
		return 1;
	}
	}
	renameFolderA(root,cdr,nam);
	cdir=getFirst(cdir);
	string mask = "";
	if (cdir!="/") mask="/";
	cdir=cdir+mask+argv[1];
	if (argc == 3) cdir=getFirst(cdr);//for avoid...
	return 0;
} 

int pare(int argc, vector<string> argv) {
	if (argc == 1||(argc == 2 && argv[1] == "view")) {
		printf("%15s\n","ID");
		//printf("%15s%15d Bytes\n","[Unallocate]",d.unallocated_size);
		for (int i = 0; i < d.partsz.size(); i++) {
			printf("%15d\n",d.partsz[i]->parname);
		}
		return 0;
	}
	if (argv[1] == "noroot") {
		string cmd;
		do {
			cmd=getl();
			_call_pare(cmd);
		} while (cmd!="exit");
	} 
	//if (argc < 2) return 0;
	if (argc < 2) {
		cout << "Required parameter missing" << endl;
		return 2;
	}
	if (argv[1] == "mount") {
		if (argc < 4) {
			cout << "Required parameter missing" << endl;
			return 2;
		}
		int target = atoi(argv[2].c_str());
		if (target < 0) {
			cout << "Specified partition does not exist or unsupported" << endl;
			return 4;
		}
		string cdr = getRealDir(argv[3]);
		for (vector<partition*>::iterator i = d.partsz.begin(); i != d.partsz.end(); i++) {
			if ((*i)->parname == target) {
				mountPartition((*i),resolve(getFirst(cdr),root),getLast(cdr));
				cout << "Operation completed successfully" << endl;
				return 0;
			}
		}
		cout << "Specified partition does not exist or unsupported" << endl;
		return 4;
	}
	if (argv[1] == "new") {
		if (d_lastname == 0) {
			createPartition(&d,-1,1);
		}
		if (createPartition(&d,++d_lastname,-1)==NULL) { // size: atoi(argv[2].c_str()
			cout << "Cannot create partition" << endl;
			return 3;
		}
		cout << "Operation completed successfully" << endl;
		return 0;
	}
	if (argv[1] == "format") {
		int target = atoi(argv[2].c_str());
		if (target < 0) {
			cout << "Specified partition does not exist or unsupported" << endl;
			return 4;
		}
		for (vector<partition*>::iterator i = d.partsz.begin(); i != d.partsz.end(); i++) {
			if ((*i)->parname == target) {
				d.partsz[target]->formatted = true;
				map<string,string> emptyfiles;
				map<string,fdirnode*> emptydirnode;
				emptydirnode["."]=d.partsz[target]->proot;
				emptydirnode[".."]=d.partsz[target]->proot->parent;
				d.partsz[target]->proot->files = emptyfiles;
				d.partsz[target]->proot->subdir = emptydirnode;
				//Sleep(d.partsz[target]->size / 100);
				cout << "Operation completed successfully" << endl;
				return 0;
			}
		}
		cout << "Specified partition does not exist or unsupported" << endl;
		return 4;
	}
	if (argv[1] == "delete") {
		int target = atoi(argv[2].c_str());
		if (target < 0) {
			cout << "Specified partition does not exist or unsupported" << endl;
			return 4;
		}
		//d.partsz.erase(d.partsz.begin()+target);//system reserved place saved
		for (vector<partition*>::iterator i = d.partsz.begin(); i != d.partsz.end(); i++) {
			if ((*i)->parname == target) {
				d.unallocated_size+=(*i)->size;
				d.partsz.erase(i);
				cout << "Operation completed successfully" << endl;
				return 0;
			}
		}
		cout << "Specified partition does not exist or unsupported" << endl;
		return 4;
	}
}

int _call_pare(string cmdline) {
	vector<string> argz;
			argz = split_arg(cmdline,true);
			argz.insert(argz.begin(),"par");
			pare(argz.size(),argz);
	return errval;
}

int apack(int argc, vector<string> argv) {
	if (argc < 2 || r.appalist.count(argv[1])==0 ) {
		cout << "Specified package does not exist" << endl;
		return 1;
	}
	cout << "Installing pack " << argv[1] << " ...  0 %";
	for (int i = 0; i < r.appalist[argv[1]].appack_size; i++) {
		printf("\b\b\b\b%2d %%",int((i*100)/r.appalist[argv[1]].appack_size));
		//Sleep(1000);
	}
	r.appalist[argv[1]].install_stat = true;
	printf("\b\b\b\bOK\n");
	return 0;
}

int chroot(int argc, vector<string> argv) {
	if (argc < 2) {
		cout << "Required parameter missing" << endl;
		return 2;
	}
	int target = atoi(argv[1].c_str());
	for (int i = 0; i < d.partsz.size(); i++) {
			if (d.partsz[i]->parname == target) {
				root = d.partsz[i]->proot;
				cdir = "/";//reset cdir
				return 0;
			}
		}
	cout << "Specified partition does not exist" << endl;
	return 1;
}

int _call_chroot(string cmdline) {
	vector<string> argz;
			argz = split_arg(cmdline,true);
			argz.insert(argz.begin(),"chroot");
			chroot(argz.size(),argz);
	return errval;
}

void initalize(void) {
	// default :)
	d = createDisk(104857600);
	createPartition(&d,-1,327680);//reserved
	createPartition(&d,1,52428800);//default partition
	//rootInit(root);
	root = d.partsz[1]->proot;
	ac=getAccounts();
	r=getDefaultAppacks();
	f["run"]=runscript;
	f["echo"]=echo;
	f["clear"]=_clear;
	f["whoami"]=whoami;
	f["errval"]=errvala;
	f["ls"]=ls;
	f["dir"]=ls; 
	f["put"]=write;
	f["write"]=write;
	f["cat"]=type; 
	f["type"]=type;
	f["cp"]=copy;
	f["copy"]=copy;
	f["mv"]=move;
	f["move"]=move;
	f["rm"]=del;
	f["del"]=del;
	f["md"]=mkdir;
	f["mkdir"]=mkdir;
	f["cd"]=cd;
	f["color"]=colors;
	f["svt"]=svt;
	f["edit"]=notepad;
	f["notepad"]=notepad;
	f["rand"]=random;
	f["help"]=help;
	f["rd"]=rd;
	f["ren"]=ren;
	f["sedit"]=seditor;
	f["par"]=pare;
	f["apack"]=apack;
	f["chroot"]=chroot;
	f["wordpad"]=sword;
}

#define KERNEL_VER "3.2.1.129"
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
		if (cmd=="") {
			continue;
		} 
		execute_command(cmd);
	}
}

int main() {
	// debugging management
	extcall["printf"]="echo";
	extcall["greet"]="echo hello world";
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
