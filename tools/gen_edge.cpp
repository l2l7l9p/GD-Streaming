#include<bits/stdc++.h>
using namespace std;

static std::mt19937 prg(time(0));

int main(int argc,char *argv[]) {
	int n=atoi(argv[1]);
	double p=atof(argv[2]);
	
	std::uniform_real_distribution<double> u(0.0, 1.0);
	
	// generate the edge list
	ofstream f("edge_list.csv");
	f << "u,v\n";
	for(int i=1; i<n; i++)
		for(int j=i+1; j<=n; j++)
			if (u(prg)<=p)
				f << i << ',' << j << '\n';
}