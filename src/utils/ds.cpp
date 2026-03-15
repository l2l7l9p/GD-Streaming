#include<bits/stdc++.h>
#include"ds.h"
using namespace std;

FastSet::FastSet(int n) {
	size=n;
	lst.resize(n);
	pos.resize(n+1);
	for(int i=0; i<n; i++) lst[i]=i+1, pos[i+1]=i;
}

bool FastSet::contain(int x) {return pos[x]!=-1;}

void FastSet::remove(int x) {
	pos[lst.back()]=pos[x];
	lst[pos[x]]=lst.back();
	pos[x]=-1;
	lst.pop_back();
	size--;
}

void FastSet::rebuild_pos() {
	for(int i=0; i<size; i++) pos[lst[i]]=i;
}

void FastSet::clear() {
	size=0;
	lst.clear();
	fill(pos.begin(), pos.end(), -1);
}