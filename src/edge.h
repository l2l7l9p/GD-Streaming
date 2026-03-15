/*
 * Edge list abstraction.
 *
 * This header defines:
 *   - Edge: a lightweight (x,y) pair (1-indexed vertex ids are assumed throughout).
 *   - Edge_list: an interface supporting multiple scans over the same edge sequence.
 *   - Two concrete implementations:
 *       * Edge_list_local_plaintxt: reads a plaintext edge list file
 *       * Edge_list_local_binary:  reads a binary edge list file
 *
 * One can implement any other edge list as long as it satisfies the requirement of Edge_list,
 * e.g., obtaining edge list from a server.
 */

#ifndef EDGE
#define EDGE

#include<bits/stdc++.h>
using namespace std;

typedef unsigned long long ULL;

const int MAX_BUFF=262144; // buffer length (ints) for fast binary reading

struct Edge{ // undirected edge
	int x, y; // endpoints; -1 indicates end-of-stream sentinel
	
	Edge(int xx=-1, int yy=-1);
	Edge reverse();
	ULL hash();
	static ULL hash(int x, int y);
};

class Edge_list {
	public:
	string edge_list_file;
	int passes; // number of completed full scans (passes) over the edge list
	ifstream fr;
	
	virtual Edge initial_get_edge()=0; // start a new pass and return the first edge (or x=-1)
	virtual Edge next_edge()=0; // return next edge within the current pass (or x=-1 when EOF)
		// usage of scanning the edge list:  for(Edge e=edgelist->initial_get_edge(); e.x!=-1; e=edgelist->next_edge()) {}
};

class Edge_list_local_plaintxt : public Edge_list {
	public:
	
	Edge_list_local_plaintxt(string file_name);
	Edge initial_get_edge();
	Edge next_edge();
};

class Edge_list_local_binary : public Edge_list {
	public:
	
	Edge_list_local_binary(string file_name);
	Edge initial_get_edge();
	Edge next_edge();
	
	private:
	int buffer[MAX_BUFF], buff_head, buff_tail;
};

#endif