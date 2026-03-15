/*
 * Logging
 */

#ifndef LOGGING
#define LOGGING

#include<bits/stdc++.h>
using namespace std;

enum Logging_Levels {
	DEBUG, INFO, WARNING, ERROR
};

class Logging {
	public:
	Logging_Levels loglevel;
	ofstream logfile;
};

void logging_init(string filename="logs.log", Logging_Levels level=INFO);
void logging(Logging_Levels level, string message);


template<typename T>
string vec2str(vector<T> &v) {
	if (v.empty()) return string();
	ostringstream oss;
	oss << '[';
	copy(v.begin(),v.end()-1,ostream_iterator<T>(oss,", "));
	oss << v.back() << ']';
	return oss.str();
}

template<typename T>
vector<T> str2vec(string &s) {
	vector<T> re;
	istringstream iss(s);
	for(string tmp; getline(iss, tmp, ' '); ) {
		T val;
		istringstream tmpss(tmp);
		tmpss >> val;
		re.push_back(val);
	}
	return re;
}

#endif