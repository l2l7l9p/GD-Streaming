#include<bits/stdc++.h>
#include"edge.h"

Edge::Edge(int xx, int yy) {
	x=xx, y=yy;
}

Edge Edge::reverse() {
	return Edge(y,x);
}

ULL Edge::hash() {
	return (x<=y) ?hash(x,y) :hash(y,x) ;
}

ULL Edge::hash(int x,int y) {
	return (((ULL)x)<<32)|y;
}

// *********** Edge_list_local_plaintxt

Edge_list_local_plaintxt::Edge_list_local_plaintxt(string file_name) {
	edge_list_file=file_name;
	passes=0;
}

Edge Edge_list_local_plaintxt::initial_get_edge() {
	passes++;
	fr.open(edge_list_file);
	string s;
	getline(fr,s);
	return next_edge();
}

Edge Edge_list_local_plaintxt::next_edge() {
	Edge e;
	char c;
	if (!(fr >> e.x >> c >> e.y)) fr.close();
	return e;
}

// *********** Edge_list_local_binary

Edge_list_local_binary::Edge_list_local_binary(string file_name) {
	edge_list_file=file_name;
	passes=0;
}

Edge Edge_list_local_binary::initial_get_edge() {
	passes++;
	fr.open(edge_list_file, ios::binary);
	buff_head=buff_tail=0;
	return next_edge();
}

Edge Edge_list_local_binary::next_edge() {
	if (buff_head>=buff_tail) {
		fr.read(reinterpret_cast<char*>(&buffer), sizeof(buffer));
		buff_head=0;
		buff_tail=fr.gcount()/sizeof(int);
		if (buff_tail==0) {
			fr.close();
			return Edge(-1,-1);
		}
	}
	Edge e(buffer[buff_head], buffer[buff_head+1]);
	buff_head+=2;
	return e;
}