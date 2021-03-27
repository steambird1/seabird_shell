#include <iostream>
#include <string>
#include "../filesystem.hpp"
using namespace std;

int main() {
	string s = "a.tar.gz";
	cout << getName(s,true) << endl;
	cout << getName(s,false) << endl;
	cout << getExt(s,true) << endl;
	cout << getExt(s,false) << endl;
	return 0;
}
