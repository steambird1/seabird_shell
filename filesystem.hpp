/*
 * Seabird Galactic Filesystem by 2021 seabird.
*/

#include <vector>
#include <map>
#include <string>
#include <cstdio>
#include <bits/stl_pair.h>
#include "accounts.hpp"
using namespace std;

#ifndef _SG_FILESYSTEM
#define _SG_FILESYSTEM

#define _SG_FSVER 202101L

#define P_READ 4
#define P_WRITE 2
#define P_EXEC 1

// First, administrator always have highest permission (7).
// Second, creater always have highest permission (7), and others will be 4.

#define ADMIN_PERM 7
#define CREATE_PERM 7
#define DEFAULT_PERM 4



typedef map<account,int> permission;

struct fdirnode {
	string this_name;
	fdirnode *parent;
	map<account,int> dir_perm;
	map<string,fdirnode*> subdir;
	map<string,pair<string,map<account,int> > > files;
	bool delete_symbol;
	int *_resv_filesize;
};

int isHavePerm(fdirnode dn, account ac, string pcd_file) {
	if (ac.account_premission > 0) return ADMIN_PERM; 
	if (pcd_file == "") {
		if (dn.parent==NULL) return CREATE_PERM; // everyone can change root! 
		if (dn.dir_perm.count(ac)) return dn.dir_perm[ac];
		else return DEFAULT_PERM;
	} else {
		if (dn.files.count(pcd_file)) {
			if (dn.files[pcd_file].second.count(ac)) return dn.files[pcd_file].second[ac];
			else return DEFAULT_PERM; 
		} else {
			return 0; // you must have no permission
		}
	}
}
inline bool isNotHavingPerm(fdirnode dn, account ac, string pcd_file, int perm) {
	return !(isHavePerm(dn,ac,pcd_file)&perm);
}

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
	map<string,fdirnode*> mf;
	mf["."]=f;
	mf[".."]=tparent;
	f->subdir=mf;
	return f;
}

inline bool isSubdirExists(const string dirname,const fdirnode *dir) {
	if (dir == NULL) return false;
	return dir->subdir.count(dirname);
}

inline bool isFileExists(const string dirname,const fdirnode *dir) {
	if (dir == NULL) return false;
	return dir->files.count(dirname);
}

/*
 * parameter: path (likes '/folder', must be folder), root of entire filesystem
 * return: folder node
*/

string getName(const string filename,const bool fullname) {
	int i;
	if (fullname) {
		for (i = filename.length()-1; i >= 0; i--) {
			if (filename[i]=='.') break;
		}
	} else {
		for (i = 0; i < filename.length(); i++) {
			if (filename[i]=='.') break; 
		}
	}
	return filename.substr(0,i);
}

string getExt(const string filename,const bool fullname) {
	int i;
	if (fullname) {
		for (i = filename.length()-1; i >= 0; i--) {
			if (filename[i]=='.') break;
		}
	} else {
		for (i = 0; i < filename.length(); i++) {
			if (filename[i]=='.') break; 
		}
	}
	return filename.substr(i+1,filename.length());
}

string getFirst(const string path) {
	string folder_part = "",finale = "/";
	if (path=="/") return "/";
	bool flag = false;
	for (int i = 1; i < path.length(); i++) {
		if (path[i]=='/') {
			finale = finale + folder_part + "/";
			folder_part = "";
			flag = true;
		} else {
			folder_part = folder_part + path[i];
		}
	}
	if (finale!="/") finale.erase(finale.end()-1);
	return finale;
}

string getLast(const string path) {
	string folder_part = "",finale = "/";
	if (path=="/") return "/";
	int fskip=0;
	if (path[0]=='/') fskip=1;
	for (int i = fskip; i < path.length(); i++) {
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

int SGRmDir(fdirnode *fd, account curlogin) {
	if (isNotHavingPerm(*fd,curlogin,"",2)) return 0;
	fd->delete_symbol = true;
	cleanup(fd->parent);
	return 1;
}

int SGRmFile(fdirnode *dir,const string filename,account curlogin) {
	if (!isFileExists(filename,dir)) return 0;
	if (isNotHavingPerm(*dir,curlogin,filename,2)) return 0;
	dir->files.erase(filename);
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

fdirnode* SGCreateFolder(fdirnode *father,const string dir_name,account curlogin) {
	if (isNotHavingPerm(*father,curlogin,"",2)) return NULL;
	fdirnode *f;
	f = newFNode(dir_name,father);
	f->dir_perm[curlogin]=CREATE_PERM;
	pullDir(father,f);
	return f;
} 

int SGRenameFolder(fdirnode *old_folder,const string dir_name,account curlogin) {
	if (isNotHavingPerm(*old_folder,curlogin,"",2)) return 0;
	if (dir_name=="") return 0;
	map<string,fdirnode*> am;
	am = old_folder->parent->subdir;
	am.erase(old_folder->this_name); 
	old_folder->this_name=dir_name;
	am[dir_name]=old_folder;
	old_folder->parent->subdir = am;
	return 1;
}

// YOU SHOULD NOT CALL THIS DIRECTLY!
int SGProceedFile(fdirnode *father,const string file_name,const string file_content,account curlogin) {
	if (isNotHavingPerm(*father,curlogin,file_name,2) && father->files.count(file_name)) return 0;
	father->files[file_name].first=file_content;
	return 1;
}

inline int SGProceedFileA(fdirnode *root, const string path, const string file_name,const string file_content, account curlogin) {
	return SGProceedFile(resolve(path,root),file_name,file_content,curlogin);
}

// New Feature: Set Permission (Requires Write)
// But I can only do like a macro...

int SGSetPermission(fdirnode *father,const string file_name,int perm,account setting,account curlogin) {
	if (file_name != "" && !isFileExists(file_name,father)) return 0;
	if (isNotHavingPerm(*father,curlogin,file_name,2)) return 0;
	if (file_name == "") {
		father->dir_perm[setting]=perm;
//		return father->dir_perm;
	} else {
		father->files[file_name].second[setting]=perm;
//		return father->files[file_name].second;
	}
}

int SGCreateFile(fdirnode *father,const string file_name,const string file_content,account curlogin) {
	if (isNotHavingPerm(*father,curlogin,"",2)) return 0; // ONLY GETTING PERMISSION IF EXIST
	if (isFileExists(file_name,father)) return 0;
	SGProceedFile(father,file_name,file_content,curlogin);
	father->files[file_name].second[curlogin]=CREATE_PERM;
	return 1;
}

// Modify file
int SGModifyFile(fdirnode *dir,const string file_name,const string file_content,account curlogin) {
	if (isNotHavingPerm(*dir,curlogin,file_name,2)) return 0;
	if (!isFileExists(file_name,dir)) return 0;
	SGProceedFile(dir,file_name,file_content,curlogin);
	return 1;
}

// We need read file...
// currently I can only write this. You can just use sscanf() to do something.
string SGReadFile(fdirnode *dir,const string file_name,account curlogin) {
	if (isNotHavingPerm(*dir,curlogin,file_name,4)) return "";
	if (!isFileExists(file_name,dir)) return "";
	return dir->files[file_name].first;
}

int SGGetFileLength(fdirnode *dir,const string file_name,account curlogin) {
	if (isNotHavingPerm(*dir,curlogin,file_name,4)) return 0;
	if (!isFileExists(file_name,dir)) return -1;
	return SGReadFile(dir,file_name,curlogin).length();
}

// actually we need to move file and copy file.
// ! It'll override target if target exists!!
int SGCopyFile(fdirnode *source_dir,const string source_filename,fdirnode *dest_dir,const string dest_filename,account curlogin) {
	if (!isFileExists(source_filename,source_dir)) return 0;
	if (isNotHavingPerm(*source_dir,curlogin,source_filename,4)) return 0;
	if (isNotHavingPerm(*dest_dir,curlogin,"",2)) return 0;
	SGProceedFile(dest_dir,dest_filename,SGReadFile(source_dir,source_filename,curlogin),curlogin);
	return 1;
}

int SGMoveFile(fdirnode *source_dir,const string source_filename,fdirnode *dest_dir,const string dest_filename,account curlogin) {
	if (!isFileExists(source_filename,source_dir)) return 0;
	SGProceedFile(dest_dir,dest_filename,SGReadFile(source_dir,source_filename,curlogin),curlogin);
	return SGRmFile(source_dir,source_filename,curlogin);
}

// Also, we need to list files likes DIR or LS.
#define LIST_DIR 1
#define LIST_FILE 2

vector<string> SGListFile(fdirnode *rootdir,const int mode,account curlogin) {
	vector<string> tresult;
	if (isNotHavingPerm(*rootdir,curlogin,"",4)) return tresult; // means a empty vector
	if (mode&1) {
		map<string,fdirnode*>::iterator i;
		for (i = rootdir->subdir.begin(); i != rootdir->subdir.end(); i++) {
			tresult.push_back("[" + i->first + "]");
		}
	}
	if (mode&2) {
		map<string,pair<string,permission> >::iterator i;
		for (i = rootdir->files.begin(); i != rootdir->files.end(); i++) {
			tresult.push_back(i->first);
		}
	}
	return tresult;
}

inline int SGWriteFile(fdirnode *root,const string file_name,const string file_content,account curlogin) {
	if (!isFileExists(file_name,root)) {
		return SGCreateFile(root,file_name,file_content,curlogin);
	} else {
		return SGModifyFile(root,file_name,file_content,curlogin);
	}
}

// some easier call using root node

inline int SGWriteFileA(fdirnode *root,const string folder_path,const string file_name,const string file_content,account curlogin) {
	return SGWriteFile(resolve(folder_path,root),file_name,file_content,curlogin);
}

inline int SGGetFileLengthA(fdirnode *root,const string folder_path,const string dir_name,account curlogin) {
	return SGGetFileLength(resolve(folder_path,root),dir_name,curlogin);
}

inline int SGRenameFolderA(fdirnode *root,const string folder_path,const string dir_name,account curlogin) {
	return SGRenameFolder(resolve(folder_path,root),dir_name,curlogin);
}

inline int SGCopyFileA(fdirnode *root, const string source_path, const string source_filename, const string dest_path, const string dest_filename,account curlogin) {
	return SGCopyFile(resolve(source_path,root),source_filename,resolve(dest_path,root),dest_filename,curlogin);
}

inline int SGMoveFileA(fdirnode *root, const string source_path, const string source_filename, const string dest_path, const string dest_filename,account curlogin) {
	return SGMoveFile(resolve(source_path,root),source_filename,resolve(dest_path,root),dest_filename,curlogin);
}

inline vector<string> SGListFileA(fdirnode *root,const string path,const int mode,account curlogin) {
	return SGListFile(resolve(path,root),mode,curlogin);
}

inline string SGReadFileA(fdirnode *root,const string path,const string file_name,account curlogin) {
	return SGReadFile(resolve(path,root),file_name,curlogin);
}

inline int SGCreateFileA(fdirnode *root,const string path,const string file_name,const string file_content,account curlogin) {
	return SGCreateFile(resolve(path,root),file_name,file_content,curlogin);
}

inline int SGModifyFileA(fdirnode *root,const string path,const string file_name,const string file_content,account curlogin) {
	return SGModifyFile(resolve(path,root),file_name,file_content,curlogin);
}

inline fdirnode* SGCreateFolderA(fdirnode *root,const string path,const string dir_name,account curlogin) {
	return SGCreateFolder(resolve(path,root),dir_name,curlogin);
}

inline bool isFileExistsA(fdirnode *root,const string path,const string dirname) {
	return isFileExists(dirname,resolve(path,root));
}

inline bool isSubdirExistsA(fdirnode *root,const string path,const string dirname) {
	return isSubdirExists(dirname,resolve(path,root));
}

inline int SGRmDirA(fdirnode *root,const string path,account curlogin) {
	return SGRmDir(resolve(path,root),curlogin);
}

inline int SGRmFileA(fdirnode *root,const string path,const string filename,account curlogin) {
	return SGRmFile(resolve(path,root),filename,curlogin);
}

inline int SGSetPermissionA(fdirnode *root,const string path,const string filename,int perm,account setting,account curlogin) {
	return SGSetPermission(resolve(path,root),filename,perm,setting,curlogin);
}

inline int isHavePermA(fdirnode *root,const string path,account query,const string filename) {
	return isHavePerm(*resolve(path,root),query,filename);
}

inline int isNotHavingPermA(fdirnode *root,const string path,account query,const string filename,int perm) {
	return isNotHavingPerm(*resolve(path,root),query,filename,perm); 
}


// for more
// they are deprecated by security problem and you should NOT use them anymore.

#define rmDir(fd) SGRmDir(fd,curlogin)
#define rmDirA(root,path) SGRmDirA(root,path,curlogin)
#define rmFile(fd,name) SGRmFile(fd,name,curlogin)
#define rmFileA(root,path,name) SGRmFileA(root,path,name,curlogin)
#define createFolder(father,dirname) SGCreateFolder(father,dirname,curlogin)
#define createFolderA(root,path,dirname) SGCreateFolderA(root,path,dirname,curlogin)
#define renameFolder(old_folder,dirname) SGRenameFolder(old_folder,dirname,curlogin)
#define renameFolderA(root,path,dirname) SGRenameFolderA(root,path,dirname,curlogin)
#define createFile(father,name,content) SGCreateFile(father,name,content,curlogin)
#define createFileA(root,path,name,content) SGCreateFileA(root,path,name,content,curlogin)
#define modifyFile(dir,filename,filecontent) SGModifyFile(dir,filename,filecontent,curlogin)
#define modifyFileA(root,path,filename,filecontent) SGModifyFileA(root,path,filename,filecontent,curlogin)
#define readFile(father,name) SGReadFile(father,name,curlogin)
#define readFileA(root,path,name) SGReadFileA(root,path,name,curlogin)
#define copyFile(sd,sf,dd,df) SGCopyFile(sd,sf,dd,df,curlogin)
#define copyFileA(root,sp,sf,dp,df) SGCopyFileA(root,sp,sf,dp,df,curlogin)
#define moveFile(sd,sf,dd,df) SGMoveFile(sd,sf,dd,df,curlogin)
#define moveFileA(root,sp,sf,dp,df) SGMoveFileA(root,sp,sf,dp,df,curlogin)
#define listFile(rd,mode) SGListFile(rd,mode,curlogin)
#define listFileA(root,path,mode) SGListFileA(root,path,mode,curlogin)
#define getFileLength(dir,fn) SGGetFileLength(dir,fn,curlogin)
#define getFileLengthA(root,path,fn) SGGetFileLengthA(root,path,fn,curlogin)
#define _proceedFile(father,file_name,file_content) SGWriteFile(father,file_name,file_content,curlogin) 


// I see we need to initalize root.
void rootInit(fdirnode *root,int filesize) {
	root->delete_symbol=false;
	root->this_name="";
	root->_resv_filesize = new int;
	*(root->_resv_filesize) = filesize;
	root->parent=NULL;//Only the root's parent is NULL.
	map<string,fdirnode*> mf;
	mf["."]=root;
	mf[".."]=root;// for avoid override.
	root->subdir=mf;
}

#endif
