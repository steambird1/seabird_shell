#include <map>
#include <string>
#include <vector>
using namespace std;

#ifndef _SC_APACK
#define _SC_APACK 1 

struct appack {
	string appack_name;
	int appack_size;//size: MB, for sure but It's fake ~
//	map<string,appack> appack_depends; // It does not work anymore, so say bye-bye to yet
};

// For some reason, ...
struct appacks {
	map<string,appack> appalist;
};

appack createAppack(string appack_name,int appack_size) {
	appack a;
	a.appack_name=appack_name;
	a.appack_size=appack_size;
	return a;
}

void pushAppack(appacks *alist,string appack_name,int appack_size) {
	alist->appalist[appack_name]=createAppack(appack_name,appack_size);
}

appacks getDefaultAppacks(void) {
	appacks r;
	pushAppack(&r,"seabird-galactic-kernel",125);
	pushAppack(&r,"seabird-galactic-login",3);
	pushAppack(&r,"seabird-galactic-security",2);
	pushAppack(&r,"seabird-galactic-shell",4);
	pushAppack(&r,"seabird-galactic-helpdoc",19);
	pushAppack(&r,"seabird-galactic-builtin-commands",8); 
	pushAppack(&r,"seabird-galactic-filesystem-commands",61);
	pushAppack(&r,"seabird-galactic-editor",11);
	pushAppack(&r,"seabird-galactic-shelltools",29);
	// and so on ...
	// I think I need 1 sec to proceed 1 MB.
	// By this way we need 1GB to install entire OS
	// and 1GB for boot-up, 1GB for partition info.
	return r;
}

#endif
