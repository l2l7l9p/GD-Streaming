/*
 * Useful data structures
 */

#ifndef DATASTRUCTURE
#define DATASTRUCTURE

#include<bits/stdc++.h>
using namespace std;

// A data structure initially containing 1..n,
// supporting O(1) look-up, O(1) remove, linear time enumeration (by enumerating lst) and linear memory.
// This is to substitute std::unordered_set<int> due to its large memory consumption
class FastSet {
	public:
	int size;
	vector<int> lst,pos;
	
	FastSet(int n);
	bool contain(int x);
	void remove(int x);
	void rebuild_pos();
	void clear();
};

#endif