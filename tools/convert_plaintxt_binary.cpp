#include<bits/stdc++.h>
using namespace std;

int main(int argc, char *argv[]) {
	string op(argv[1]), name1(argv[2]), name2(argv[3]);
	if (op.compare("pb")==0) {
		// plaintxt -> binary
		ifstream fr(name1);
		ofstream fw(name2, ios::binary);
		string s;
		int x,y;
		char c;
		getline(fr,s);
		while (fr >> x >> c >> y) {
			fw.write(reinterpret_cast<char*>(&x), sizeof(int));
			fw.write(reinterpret_cast<char*>(&y), sizeof(int));
		}
	} else if (op.compare("bp")==0) {
		// binary -> plaintxt
		ifstream fr(name1, ios::binary);
		ofstream fw(name2);
		int x[2];
		fw << "u,v\n";
		while (fr.read(reinterpret_cast<char*>(x), sizeof(int)<<1)) {
			fw << x[0] << ',' << x[1] << '\n';
		}
	} else {
		cout << "Error: only plaintxt->binary (pb) and binary->plaintxt (bp) are supported" << endl;
	}
}