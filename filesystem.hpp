/*
 * Seabird Galactic Filesystem by 2021 seabird.
*/

#include <vector>
#include <map>
#include <string>
#include <cstdio>
using namespace std;

#ifndef _SG_FILESYSTEM
#define _SG_FILESYSTEM

#define _SG_FSVER 202101L

struct fdirnode {
	string this_name;
	fdirnode *parent;
	map<string,fdirnode*> subdir;
	map<string,string> files;
	bool delete_symbol;
};

fdirnode createFNode(const string dir_name, fdirnode *tparent) {
	fdirnode f;
	f.this_name=dir_name;
	f.parent=tparent;
	f.delete_symbol=false;
	return f;
}

fdirnode* newFNode(const string dir_name, fdirnode *tparent) {
	fdirnode *f = new fdirnode;
	f->this_name=dir_name;
	f->parent=tparent;
	f->delete_symbol=false;
	return f;
}

inline bool isSubdirExists(const string dirname,const fdirnode *dir) {
	return dir->subdir.count(dirname);
}

inline bool isFileExists(const string dirname,const fdirnode *dir) {
	return dir->files.count(dirname);
}

/*
 * parameter: path (likes '/folder', must be folder), root of entire filesystem
 * return: folder node
*/

string getFirst(const string path) {
	string folder_part = "",finale = "/";
	if (path=="/") return "/";
	for (int i = 1; i < path.length(); i++) {
		if (path[i]=='/') {
			finale = finale + folder_part;
			folder_part = "";
		} else {
			folder_part = folder_part + path[i];
		}
	}
	return finale;
}

string getLast(const string path) {
	string folder_part = "",finale = "/";
	if (path=="/") return "/";
	for (int i = 1; i < path.length(); i++) {
		if (path[i]=='/') {
			finale = finale + folder_part;
			folder_part = "";
		} else {
			folder_part = folder_part + path[i];
		}
	}
	return folder_part;
}


fdirnode* resolve(const string path,fdirnode *root) {
	string folder_part = "";
	fdirnode *curs;
	curs = root;
	if (path=="/") return root;
	for (int i = 1; i < path.length(); i++) {
		if (path[i]=='/') {
			if (!isSubdirExists(folder_part,curs)) return NULL;
			curs = curs->subdir[folder_part];
			folder_part = "";
		} else {
			folder_part = folder_part + path[i];
		}
	}
	//printf("%s\n%s\n",folder_part.c_str(),curs->this_name.c_str());
	if (!isSubdirExists(folder_part,curs)) return NULL;
	return curs->subdir[folder_part];
}

/*
 * parameter: folder node
 * do: clean deleted folder node
 * return: cleaned node 
*/

// remix all like this!
void cleanup(fdirnode *dnode) {
	map<string,fdirnode*> dnode_fmap;
	dnode_fmap = dnode->subdir;
	map<string,fdirnode*>::iterator i;
	for (i = dnode_fmap.begin(); i != dnode_fmap.end(); i++) {
		if (i->second->delete_symbol) dnode_fmap.erase(i++);
		if (dnode_fmap.empty()) break;
	}
	fdirnode f;
	f = createFNode(dnode->this_name,dnode->parent);
	f.files=dnode->files;
	f.subdir=dnode_fmap;
	*dnode = f;
}

/*
 * parameter: folder node
 * do: delete this!
 * actually, It'll do cleanup.
*/

void rmDir(fdirnode *fd) {
	fd->delete_symbol = true;
	cleanup(fd->parent);
}

/*
 * actually, you should run like dir=rmfile(dir,filename) to complete deletion.
*/
int rmFile(fdirnode *dir,const string filename) {
	if (!isFileExists(filename,dir)) return 0;
	fdirnode f;
	f = createFNode(dir->this_name,dir->parent);
	f.subdir=dir->subdir;
	map<string,string> nf;
	nf = dir->files;
	nf.erase(filename);
	f.files=nf;
	*dir = f;
	return 1;
}
/*
 * parameter: parent folder
 * do: create something!
 *
*/

// Can we just modify the node?
int pullDir(fdirnode *father, fdirnode *son) {
	// pull directory.
	son->parent = father;
	map<string,fdirnode*> mfd;
	mfd = father->subdir;
	if (mfd.count(son->this_name)) return 0;
	mfd[son->this_name]=son;
	fdirnode f;
	f = createFNode(father->this_name,father->parent);
	f.files=father->files;
	f.subdir=mfd;
	*father = f;
	return 1;
}

// As this algorithm I can only return an address.
// return: created folder node.

fdirnode* createFolder(fdirnode *father,const string dir_name) {
	fdirnode *f;
	f = newFNode(dir_name,father);
	pullDir(father,f);
	return f;
} 

void _proceedFile(fdirnode *father,const string file_name,const string file_content) {
	fdirnode f;
	f = createFNode(father->this_name,father->parent);
	f.subdir=father->subdir;
	map<string,string> fdr;
	fdr = father->files;
	fdr[file_name] = file_content;
	f.files = fdr;
	*father = f;
}

int createFile(fdirnode *father,const string file_name,const string file_content) {
	if (isFileExists(file_name,father)) return 0;
	_proceedFile(father,file_name,file_content);
	return 1;
}

// Modify file
int modifyFile(fdirnode *dir,const string file_name,const string file_content) {
	if (!isFileExists(file_name,dir)) return 0;
	_proceedFile(dir,file_name,file_content);
	return 1;
}

// We need read file...
// currently I can only write this. You can just use sscanf() to do something.
string readFile(fdirnode *dir,const string file_name) {
	if (!isFileExists(file_name,dir)) return "";
	return dir->files[file_name];
}

// actually we need to move file and copy file.
// ! It'll override target if target exists!!
int copyFile(fdirnode *source_dir,const string source_filename,fdirnode *dest_dir,const string dest_filename) {
	if (!isFileExists(source_filename,source_dir)) return 0;
	_proceedFile(dest_dir,dest_filename,readFile(source_dir,source_filename));
	return 1;
}

int moveFile(fdirnode *source_dir,const string source_filename,fdirnode *dest_dir,const string dest_filename) {
	if (!isFileExists(source_filename,source_dir)) return 0;
	_proceedFile(dest_dir,dest_filename,readFile(source_dir,source_filename));
	return rmFile(source_dir,source_filename);
}

// Also, we need to list files likes DIR or LS.
#define LIST_DIR 1
#define LIST_FILE 2

vector<string> listFile(fdirnode *rootdir,const int mode) {
	vector<string> tresult;
	if (mode&1) {
		map<string,fdirnode*>::iterator i;
		for (i = rootdir->subdir.begin(); i != rootdir->subdir.end(); i++) {
			tresult.push_back("[" + i->first + "]");
		}
	}
	if (mode&2) {
		map<string,string>::iterator i;
		for (i = rootdir->files.begin(); i != rootdir->files.end(); i++) {
			tresult.push_back(i->first);
		}
	}
	return tresult;
}

// At this moment, we need a "root" node.

inline int copyFileA(fdirnode *root, const string source_path, const string source_filename, const string dest_path, const string dest_filename) {
	return copyFile(resolve(source_path,root),source_filename,resolve(dest_path,root),dest_filename);
}

inline int moveFileA(fdirnode *root, const string source_path, const string source_filename, const string dest_path, const string dest_filename) {
	return moveFile(resolve(source_path,root),source_filename,resolve(dest_path,root),dest_filename);
}

inline vector<string> listFileA(fdirnode *root,const string path,const int mode) {
	return listFile(resolve(path,root),mode);
}

inline string readFileA(fdirnode *root,const string path,const string file_name) {
	return readFile(resolve(path,root),file_name);
}

inline int createFileA(fdirnode *root,const string path,const string file_name,const string file_content) {
	return createFile(resolve(path,root),file_name,file_content);
}

inline int modifyFileA(fdirnode *root,const string path,const string file_name,const string file_content) {
	return modifyFile(resolve(path,root),file_name,file_content);
}

inline fdirnode* createFolderA(fdirnode *root,const string path,const string dir_name) {
	return createFolder(resolve(path,root),dir_name);
}

inline bool isFileExistsA(fdirnode *root,const string path,const string dirname) {
	return isFileExists(dirname,resolve(path,root));
}

inline bool isSubdirExistsA(fdirnode *root,const string path,const string dirname) {
	return isSubdirExists(dirname,resolve(path,root));
}

inline void rmDirA(fdirnode *root,const string path) {
	rmDir(resolve(path,root));
}

inline int rmFileA(fdirnode *root,const string path,const string filename) {
	return rmFile(resolve(path,root),filename);
}

// I see we need to initalize root.
void rootInit(fdirnode *root) {
	root->delete_symbol=false;
	root->this_name="";
	root->parent=NULL;//Only the root's parent is NULL.
}

#endif
