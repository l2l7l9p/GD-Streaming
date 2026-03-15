/*
 * Minimal config used by main.cpp.
 * The input config file uses 'key: value' lines.
 * (It is not a full YAML parser despite the function name.)
 */

#ifndef CONFIG
#define CONFIG

#include<bits/stdc++.h>
using namespace std;

struct Config {
	int n,k,target,MAX_EDGES;
	double epsilon,es_c=0.1;
	string edgelistfile,edgelistmode,ddordermode,ddorderfile,func;
};

Config load_yaml(string file); // parse config file into a Config struct

#endif