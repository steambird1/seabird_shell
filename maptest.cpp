#include <map>
#include <iostream>
using namespace std;

struct w {
	int* b;
};

struct ww {
	int b;
};

int * zzz(void) {
	int a = 5;
	return &a;
} 

void zzzz(int *a) {
	(*a)=(*(zzz()));
}

int main() {
	map<int,int*> a;
	int *p,b = 4;
	p = &b;
	a[b]=p;
	cout<<*(a[b])<<endl;
	map<int,int*> q;
	q=a;
	cout<<*(q[b])<<endl;
	
	ww wa,wb;
	wa.b=5;
	wb=wa;
	cout<<wb.b<<endl;
	
	cout<<*(zzz())<<endl;
	
	zzzz(p);
	cout<<*(p)<<endl;
	return 0;
} 
