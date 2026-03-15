#include<bits/stdc++.h>
#include"all_graphlets.h"
#include"utils/logging.h"
using namespace std;

tuple<vector<int>,vector<int>> get_all_graphlets(int k) {
	logging(INFO,"Computing all graphlets of size "+to_string(k)+"...");
	
	const int MAX_K=15;				// It is safe to assume that k<=15
	int MAX_ADJ=(1<<(k*(k-1)/2))-1;	// possible adjacency matrices: 0 ~ MAX_ADJ
	vector<int> p0(k);				// initial permutation
	for(int i=0; i<k; i++) p0[i]=i;
	int bit_pos[MAX_K][MAX_K], bit_pos_count=0;
	for(int i=k-2; i>=0; i--)
		for(int j=k-1; j>i; j--) bit_pos[i][j]=bit_pos[j][i]=bit_pos_count++;
	vector<int> graphlets;
	vector<int> mapping(MAX_ADJ+1);
	
	for(int adj=0; adj<=MAX_ADJ; adj++) {
		// check connectivity
		int dsu[MAX_K];
		std::function<int(int)> get_root=[&](int x) {
			return dsu[x]==x ?x :dsu[x]=get_root(dsu[x]) ;
		};
		for(int i=0; i<k; i++) dsu[i]=i;
		for(int i=0; i<k; i++)
			for(int j=i+1; j<k; j++) if ((adj>>(bit_pos[i][j]))&1) dsu[get_root(i)]=get_root(j);
		for(int i=0; i<k; i++) get_root(i);
		if (count(dsu,dsu+k,dsu[0])!=k) continue;
		
		// minimal representation
		int min_adj=adj;
		vector<int> p=p0;
		do {
			int new_adj=0;
			for(int i=0; i<k; i++)
				for(int j=i+1; j<k; j++) (new_adj<<=1)|=((adj>>bit_pos[p[i]][p[j]])&1);
			min_adj=min(min_adj, new_adj);
		} while (next_permutation(p.begin(),p.end()));
		if (min_adj==adj) {
			graphlets.push_back(adj);
			mapping[adj]=graphlets.size()-1;
		} else mapping[adj]=mapping[min_adj];
	}
	return {graphlets,mapping};
}