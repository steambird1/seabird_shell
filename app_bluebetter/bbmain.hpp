#include "../shellexec.hpp"
#include <string>
#include <vector>
#include <map>
#include <cstdio>
using namespace std;

#ifndef _BLUEBETTER_
#define _BLUEBETTER_

#define _BLUEBETTER_VER 202104L

// runCode() to run BlueBetter Code.

// Debug mode
const bool debug = false;
#define ifdebug if(debug)

// Shellexecs
typedef vector<string> splits;

// BlueBetter CPU types
typedef int ptr_t;
typedef int memval_t;

typedef map<ptr_t,memval_t> bmemap;

// Line IDs
typedef int lineid_t;
typedef string lineflag_t;

typedef map<lineflag_t,lineid_t> jmpmap;

// Error IDs
const int no_command = 1;
const int required_parameter = 2;
const int bad_jump = 3; // override or not exists
const int bad_tell = 4;
const int math_error = 5; 

// Macros
#define throws(err) mem[-1]=err; continue; 
#define check_parameter(count) if (asplit.size() < count) {throws(required_parameter);}

int getRealVal(bmemap* memap, string ptr) {
	if (ptr[0]=='%'||ptr[0]=='&') { 
		int ptr_p = atoi(ptr.substr(1,ptr.length()).c_str());
		if (!memap->count(ptr_p)) (*memap)[ptr_p]=0;
		int pval = (*memap)[ptr_p];
		if (ptr[0]=='%') return pval;
		else if (ptr[0]=='&') {
			if (!memap->count(pval)) (*memap)[pval]=0;
			return (*memap)[pval];
		}
	} else {
		return atoi(ptr.c_str());
	}
};

#define _op() check_parameter(3); int ptr = getRealVal(&mem,asplit[1]), val = getRealVal(&mem,asplit[2]); if (mem.count(ptr)==0) mem[ptr]=0
#define _cop() check_parameter(4); int dst = getRealVal(&mem,asplit[1]), ptr1 = getRealVal(&mem,asplit[2]), ptr2 = getRealVal(&mem,asplit[3])

// runCode function
int runCode(string bcmd) {
	splits lsplit = spiltLines(bcmd);
	splits asplit;//arguments
	bmemap mem;
	jmpmap jmps;
	ifdebug printf("Resolving...\n"); 
	for (int i = 0; i < lsplit.size(); i++) {
		asplit = split_arg(lsplit[i],false);
		if (asplit.size() < 1) continue;//ignore
		if (asplit[0]=="bflg") {
			if (asplit.size() < 2) return -1; // invaild symbol
			jmps[asplit[1]]=i; // should move to first
		}
	}
	for (int i = 0; i < lsplit.size(); i++) {
		asplit = split_arg(lsplit[i],false);
		if (asplit.size() < 1) continue; // no command already
		if (asplit[0][0]==';') {
			continue;
		} else if (asplit[0]=="bjmp") {
			check_parameter(3);
			int ptrz = getRealVal(&mem,asplit[1]);
			if (!jmps.count(asplit[2])) {
				throws(bad_jump);
			}
			if (ptrz!=0) {
				i = jmps[asplit[2]];
			}
		} else if (asplit[0]=="bset") {
			check_parameter(3);
			int val = getRealVal(&mem,asplit[2]), ptr = atoi(asplit[1].c_str());
			mem[ptr]=val;
		} else if (asplit[0]=="tand") {
			_op(); mem[ptr]=mem[ptr]&val;
		} else if (asplit[0]=="band") {
			_op(); mem[ptr]=mem[ptr]&&val;
		} else if (asplit[0]=="tor") {
			_op(); mem[ptr]=mem[ptr]|val; 
		} else if (asplit[0]=="bor") {
			_op(); mem[ptr]=mem[ptr]||val;
		} else if (asplit[0]=="txor") {
			_op(); mem[ptr]=mem[ptr]^val;
		} else if (asplit[0]=="bnot") {
			check_parameter(2);
			int ptr = atoi(asplit[1].c_str());
			if (!mem.count(ptr)) mem[ptr]=0;
			mem[ptr]=!mem[ptr];
		} else if (asplit[0]=="tlm") {
			_op(); mem[ptr]=mem[ptr]<<val;
		} else if (asplit[0]=="trm") {
			_op(); mem[ptr]=mem[ptr]>>val;
		} else if (asplit[0]=="badd") {
			_op(); mem[ptr]+=val;
		} else if (asplit[0]=="bded") {
			_op(); mem[ptr]-=val;
		} else if (asplit[0]=="btim") {
			_op(); mem[ptr]*=val;
		} else if (asplit[0]=="bdiv") {
			_op(); if (val==0) {
				throws(math_error);
			}
			mem[ptr]/=val;
		} else if (asplit[0]=="bmod") {
			_op(); if (val==0) {
				throws(math_error);
			}
			mem[ptr]%=val;
		} else if (asplit[0]=="bequ") {
			_cop();
			if (ptr1==ptr2) mem[dst]=1;
			else mem[dst]=0;
		} else if (asplit[0]=="bcmp") {
			_cop();
			if (ptr1>ptr2) mem[dst]=1;
			else mem[dst]=0;
		} else if (asplit[0]=="btel") {
			check_parameter(3);
			int ptr = getRealVal(&mem,asplit[2]);
			if (asplit[1]=="get") {
				mem[ptr]=int(getchar());
			} else if (asplit[1]=="put") {
				putchar(char(ptr));
			} else if (asplit[1]=="read") {
				int s;
				scanf("%d",&s);
				mem[ptr]=s;
			} else if (asplit[1]=="write") {
				printf("%d",ptr);
			} else {
				throws(bad_tell);
			}
		}
	}
	if (!mem.count(0)) return 0;
	else return mem[0]; 
}

#endif 
