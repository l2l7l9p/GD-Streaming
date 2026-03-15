#include<bits/stdc++.h>
#include"config.h"
using namespace std;

Config load_yaml(string file) {
	ifstream fr(file);
	string key,val;
	Config re;
	while (fr >> key >> val) {
		if (key.compare("n:")==0) re.n=stoi(val);
		if (key.compare("edgelistfile:")==0) re.edgelistfile=val;
		if (key.compare("edgelistmode:")==0) re.edgelistmode=val;
		if (key.compare("ddordermode:")==0) re.ddordermode=val;
		if (key.compare("ddorderfile:")==0) re.ddorderfile=val;
		if (key.compare("es_c:")==0) re.es_c=stod(val);
		if (key.compare("epsilon:")==0) re.epsilon=stod(val);
		if (key.compare("k:")==0) re.k=stoi(val);
		if (key.compare("target:")==0) re.target=stoi(val);
		if (key.compare("MAX_EDGES:")==0) re.MAX_EDGES=stoi(val);
		if (key.compare("func:")==0) re.func=val;
	}
	fr.close();
	return re;
}