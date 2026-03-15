#include<bits/stdc++.h>
using namespace std;

bool operator < (const vector<int> &a,const vector<int> &b) {
	for(int i=0; i<a.size(); i++) {
		if (a[i]<b[i]) return 1;
		if (a[i]>b[i]) return 0;
	}
	return 0;
}

bool operator == (const vector<int> &a,const vector<int> &b) {
	for(int i=0; i<a.size(); i++) if (a[i]!=b[i]) return 0;
	return 1;
}

template<typename T>
string vec2str(vector<T> &v) {
	if (v.empty()) return string();
	ostringstream oss;
	oss << '[';
	copy(v.begin(),v.end()-1,ostream_iterator<T>(oss,", "));
	oss << v.back() << ']';
	return oss.str();
}

int main(int argc,char *argv[]) {
	ifstream fr(argv[1]);
	int n,k;
	fr >> n >> k;
	vector<vector<int>> samples(n);
	for(int i=0; i<n; i++) {
		samples[i].reserve(k);
		copy_n(istream_iterator<int>(fr), k, back_inserter(samples[i]));
		sort(samples[i].begin(),samples[i].end());
	}
	
	sort(samples.begin(),samples.end());
	
	ofstream fw("probability.txt");
	int cnt=0;
	for(int i=0; i<n; i++)
		if (i==0 || samples[i]==samples[i-1]) cnt++;
			else {
				fw << vec2str(samples[i-1]) << ": " << cnt << '/' << n << endl;
				cnt=1;
			}
	fw << vec2str(samples.back()) << ": " << cnt << '/' << n << endl;
}