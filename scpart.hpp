#include <vector>
#include <cstdio>
#include "filesystem.hpp"
using namespace std;
#ifndef _SC_PARTITION
#define _SC_PARTITION 1

struct partition {
	int parname; // -1 = system reserved.
	int size;
	fdirnode *proot;
	bool formatted; 
};

struct disk {
	int full_size;
	int unallocated_size;
	vector<partition*> partsz;
};

disk createDisk(int dsize) { 
	disk d;
	d.full_size=dsize;
	d.unallocated_size=dsize;
	return d;
}

partition* createPartition(disk *inDisk, int pparname, int psize) {
	if (psize>=inDisk->unallocated_size) return NULL;
	partition *p = new partition;
	p->parname=pparname;
	p->size=psize;
	p->proot = new fdirnode;
	p->formatted = false;
	rootInit(p->proot);
	inDisk->partsz.push_back(p);
	inDisk->unallocated_size-=psize;
	return p;
}

void deletePartition(disk *inDisk,int iteratorNumber) {
	inDisk->unallocated_size+=(inDisk->partsz[iteratorNumber])->size;
	inDisk->partsz.erase((inDisk->partsz.begin())+iteratorNumber);
}

inline void mountPartition(partition *par, fdirnode *father, string pname) {
	par->proot->this_name = pname;
	pullDir(father,par->proot);
}

#endif
