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

account _createAccount(string name,int permission,string passwd) {
	account c;
	c.account_name=name;c.account_password=passwd;c.account_premission=permission;
	return c;
}

account getAdminInfo() {
	return _createAccount("admin",1,"admin");
} 

acclist getAccounts() {
	acclist c;
	c["admin"]=getAdminInfo();
	c["user"]=_createAccount("user",0,"");
	return c;
}

#endif
