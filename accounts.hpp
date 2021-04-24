#include <map>
#include <vector>
#include <string>
using namespace std;

#ifndef _ACCOUNTS_
#define _ACCOUNTS_

struct account {
	string account_name;
	int account_premission; // 0 = normal , 1 = admin
	string account_password;
}; 

typedef map<string,account> acclist;

// Supporting for map
bool operator < (account a,account b) {
	if (a.account_premission==b.account_premission) {
		return a.account_name < b.account_name;
	}
	return a.account_premission < b.account_premission; 
}

account _createAccount(string name,int permission,string passwd) {
	account c;
	c.account_name=name;c.account_password=passwd;c.account_premission=permission;
	return c;
}

account getAdminInfo() {
	return _createAccount("admin",1,"admin");
} 

account getSystem() {
	return _createAccount("system",1,"\a"); // I think you can't input '\a'.
}

acclist getAccounts() {
	acclist c;
	c["system"]=getSystem();
	c["admin"]=getAdminInfo();
	c["user"]=_createAccount("user",0,"");
	return c;
}

#endif
