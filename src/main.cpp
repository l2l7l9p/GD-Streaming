/*
 * This code serves as an example of executing the following functions:
 *   - preprocessing: compute DD order and/or the initial distribution
 *   - sampling: uniform graphlet sampling (UGS) or k-graphlet distribution (GD)
 *   - (report the number of passes used by each phase)
 */

#include<bits/stdc++.h>
#include"graph.h"
#include"all_graphlets.h"
#include"config.h"
#include"utils/logging.h"
using namespace std;

int main(int argc,char *argv[]) {
	logging_init("logs.log",INFO);
	Config config=load_yaml(string(argv[1]));
	
	// initialize (constructs Edge_list and precomputes graphlet maps)
	Graph graph(config.n, config.edgelistmode, config.edgelistfile, config.MAX_EDGES);
	
	// preprocess
	if (config.ddorderfile.empty()) {
		// preprocess by computing a dd-order
		vector<int> empty;
		graph.sampling_preprocess(config.k, config.epsilon, empty, config.ddordermode, config.es_c);
		ofstream f("DD_order.bin", ios::binary);
		// DD_order is written as n 32-bit ints (vertex labels), for reuse in later runs.
		f.write(reinterpret_cast<const char*>(graph.DD_order.data()), graph.DD_order.size()*sizeof(int));
	} else {
		// preprocess using a pre-computed dd-order
		ifstream f(config.ddorderfile, ios::binary);
		vector<int> DD_order(graph.n);
		f.read(reinterpret_cast<char*>(DD_order.data()), graph.n*sizeof(int));
		graph.sampling_preprocess(config.k, config.epsilon, DD_order);
	}
	int passes_for_preprocess=graph.edge_list->passes;
	
	if (config.func.compare("ugs")==0) {
		// uniform graphlet sampling
		vector<IntList> samples=graph.sample_ugs(config.k, config.target);
		ofstream f("samples.txt");
		// Output format: |samples|, k, then concatenation of all sampled k-vertex sets.
		f << samples.size() << ' ' << config.k << ' ';
		for(auto &sample: samples) copy(sample.begin(), sample.end(), ostream_iterator<int>(f," "));
	} else if (config.func.find("gd")!=string::npos) {
		// approximate graphlet distribution
		auto [graphlets,mapping]=get_all_graphlets(config.k);
		vector<LD> graphlet_distribution=graph.sample_gd(config.k, config.target, graphlets.size(), mapping, (config.func.find("rejection")!=string::npos));
		ofstream f("graphlet_distribution.txt");
		// Output format: one line per graphlet class: <adjacency mask> <estimated frequency>.
		for(int i=0; i<graphlets.size(); i++) f << graphlets[i] << ' ' << graphlet_distribution[i] << endl;
	}
	
	logging(INFO,"Number of passes in total = "+to_string(graph.edge_list->passes));
	logging(INFO,"Number of passes for preprocessing = "+to_string(passes_for_preprocess));
	logging(INFO,"Number of passes for sampling = "+to_string(graph.edge_list->passes - passes_for_preprocess));
}