/*
 * Random number generators
 */

#ifndef RANDOMNESS
#define RANDOMNESS

#include<bits/stdc++.h>
using namespace std;

static std::mt19937 prg(time(0));

// A single integer in the range of [a,b].
template<typename T>
T ran_int(T a,T b) {
	std::uniform_int_distribution<T> u(a,b);
	return u(prg);
}

// n differrent integers in the range of [nmin, nmax]
template<typename T>
vector<T> ran_int_range(T nmin, T nmax, int n) {
	std::uniform_int_distribution<T> u(nmin,nmax);
	vector<T> re;
	re.reserve(n);
	for(int i=0; i<n; i++) {
		int t;
		do {
			t=u(prg);
		} while (find(re.begin(), re.end(), t)!=re.end());
		re.push_back(t);
	}
	return re;
}

// A single real number in the range of [a,b].
template<typename T>
T ran_real(T a,T b) {
	std::uniform_real_distribution<T> u(a,b);
	return u(prg);
}

// n i.i.d. integers in the range of [0, weights.size()), according to weights
template<typename T>
vector<int> ran_discrete(vector<T> &weights, int n) {
	vector<int> re(n);
	std::discrete_distribution<> d(weights.begin(), weights.end());
	for(int &x:re) x=d(prg);
	return re;
}

// A single integer in the range of [0, weights.size()), according to weights
template<typename T>
int ran_discrete(vector<T> &weights) {
	std::discrete_distribution<> d(weights.begin(), weights.end());
	return d(prg);
}

#endif