#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <iostream>
#include <conio.h>
using namespace std;

#ifndef _SHELLEXEC_
#define _SHELLEXEC_

#define HOST_NAME "sg.virtShell"

typedef int(*shell_func)(int,vector<string>);
typedef map<string,shell_func> funcall;

vector<string> split_arg(string cmd,bool allow_quotes) {
	vector<string> argv;
	// argc = argv.size()
	bool inquote = false;
	string buf = "";
	for (int i = 0; i < cmd.length(); i++) {
		switch (cmd[i]) {
			case ' ': 
				if (inquote) {
					buf = buf + ' ';
					break;
				}
				argv.push_back(buf);
				buf = "";
				break;
			case '"':
				if (allow_quotes) inquote = !inquote;
				else buf = buf + '"';
				break;
			default:
				buf = buf + cmd[i];
				break;
		} 
	}
	if (buf!="") argv.push_back(buf);
	return argv;
}

int call_cmd(funcall fc_list,vector<string> argv) {
	string command = argv[0];
	if (argv.size()<1) return -127;
	if (!fc_list.count(command)) return -1;
	return fc_list[command](argv.size(),argv);
}

string pwd_input(void) {
	int k;
		string s = "";
		do {
			k=getch();
			if (k==13) break;
			if (k==8&&s.length()>0) {
				cout<<"\b \b";
				s=s.substr(0,s.length()-1);
				continue;
			}
			cout<<"*";
			s+=char(k);
		} while (1);
	return s;
}

#endif
