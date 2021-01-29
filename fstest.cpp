#include "filesystem.hpp"
#include <iostream>
using namespace std;

fdirnode *r = new fdirnode, *f = new fdirnode, *f2 = new fdirnode;//root of this testing filesystem ...

int main() {
	rootInit(r);
	f = createFolderA(r,"/","testFolder");
	f2 = createFolderA(r,"/testFolder","hello");
	if (isSubdirExistsA(r,"/","testFolder")) cout << "well done!" << endl;
	if (isSubdirExistsA(r,"/testFolder","hello")) cout << "well done!" << endl;
	createFileA(r,"/testFolder","test.txt","hello world!");
	cout << readFileA(r,"/testFolder","test.txt") << endl;
	// or deleting files?
	modifyFileA(r,"/testFolder","test.txt","welcome!");
	cout << readFileA(r,"/testFolder","test.txt") << endl;
	
	// start!
	vector<string> v;
	v = listFileA(r,"/testFolder",3);
	int a = 3&1, b = 3&2;
	cout<<a<<" "<<b<<endl<<endl;
	for (vector<string>::iterator i = v.begin(); i != v.end(); i++) cout << *i << "    " << endl;
	
	//return 0;
	rmFileA(r,"/testFolder","test.txt");
	if (isFileExistsA(r,"/testFolder","test.txt")) cout << "uh-oh..." << endl;
	//for (map<string,string>::iterator i = f->files.begin(); i != f->files.end(); i++) cout << i->first << endl;
	fdirnode *f3 = new fdirnode;
	f3 = createFolderA(r,"/testFolder/hello","hi");
	if (isSubdirExistsA(r,"/testFolder/hello","hi")) cout << "well done!" << endl;
	
	// not resolve problem
	// try this?
	//rmDir(f3); //stucked.
	//return 0;
	if (!isSubdirExistsA(r,"/testFolder","hello")) {
		cout << "emm??" << endl;
		return 0;
	}
	
	rmDirA(r,"/testFolder/hello");//stucked
	return 0;
	if (isSubdirExistsA(r,"/testFolder","hello")) cout << "uh-oh..." << endl;
	return 0;
}
