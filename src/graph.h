/*
 * Core implementation of critical algorithms.
 *
 * High-level pipeline:
 *   1) Preprocessing
 *      Usage: sampling_preprocess(k, epsilon, provided_DD_order(could be empty), DD_order_mode, c);
 *      Finer-grained steps:
 *      (a) Compute a theta = 1/(1+epsilon) degree-dominating (DD) order.
 *          Implemented by Graph::compute_DD_order(), with multiple modes:
 *            - "approx"    : ApproxDD peeling
 *            - "ES"        : edge-sampling acceleration
 *            - "heuristic" : optional in-memory heuristic, used by "approx-heuristic" or "ES-heuristic"
 *      (b) Compute the initial distribution p(v):
 *          p(v) := d(v | G(v))^{k-1} for vertices whose bucket is non-empty.
 *
 *   2) Sampling (k passes per sample batch, implemented in a pass-efficient way):
 *      - sample_ugs(): uniform graphlet sampling via rejection using Gamma/p(S).
 *      - sample_gd(): graphlet-distribution estimation via counters.
 *
 * Throughout, the graph is accessed only through Edge_list scans to model passes.
 */

#ifndef GRAPH
#define GRAPH

#include<bits/stdc++.h>
#include"edge.h"
#include"utils/ds.h"
using namespace std;

typedef long long LL;
typedef long double LD;
typedef vector<int> IntList;
typedef unordered_set<int> IntSet;
typedef unordered_map<int,int> IntMap;

class Graph {
	public:
	int n;               // number of vertices (labels assumed 1..n)
	LL MAX_EDGES;        // memory budget proxy: max number of edges we allow to materialize in RAM
	Edge_list *edge_list; // streaming edge access; each full scan increments pass_cnt
	IntList DD_order;    // computed vertex order
	
	Graph(int nn, string edgelistmode, string edge_list_file, LL me);
		// me: memory parameter (in edges) used to set MAX_EDGES and tune preprocessing.
	
	// functions for preprocessing
	void compute_DD_order(double epsilon, string mode, double c=0.1);
	void compute_distrib(int k, double epsilon);
	void sampling_preprocess(int k, double epsilon, IntList &provided_DD_order, string DD_order_mode="ES-heuristic", double c=0.1);
	
	// functions for sampling
	vector<IntList> sample_ugs(int k, int target);
		// UGS via rejection: returns 'target' accepted k-vertex sets.
	vector<LD> sample_gd(int k, int target, int graphlet_num, vector<int> &mapping, bool is_rejection);
		// GD estimation: returns counts per graphlet class (counter if is_rejection=false; rejection if true).
	
	// useful tools
	vector<LD> evaluate_DD_order();
		// Computes per-vertex epsilon_v values, returned as a list for histogramming.
	
	// for experiment only
	void sample_gd_both(int k, int target, int graphlet_num, vector<int> &mapping, vector<int> &graphlets);
	
	~Graph();
	
	
	private:
	IntList DD_ranking;   // inverse permutation: DD_ranking[v] = position of v in DD_order
	vector<LD> pr;        // initial distribution p(v)
	LD Gamma;             // lower bound Gamma <= min_S p(S) used by rejection
	
	// functions for preprocessing
	void get_degree_G(FastSet &vertices, IntList &degree_G);
	IntList get_degree_H(IntMap &H);
	void PEEL_approx(FastSet &vertices, IntList &degree_G, double epsilon, double alpha, int max_degree);
	void PEEL_bruteforce(FastSet &vertices, function<bool(int)> is_large, IntList &degree_G, double threshold_lb);
	void select_large(FastSet &vertices, unordered_map<int,IntList> &edges, double threshold, double p);
	void remove_isolated_vertices(FastSet &vertices, IntList &vertices_isolated, IntList &degree);
	
	// functions for sampling
	tuple<unordered_map<ULL,int>,IntList> get_degree_duGv(vector<IntList> &samples, int sample_size);
	vector<IntList> RAND_GROW_fast(IntList &v, int k, int batch_size);
	void PROB(vector<IntList> &samples, int k, function<void(IntList&, LD, int)> PROB_operator);
	void sample_one_batch(int k, int batch_size, function<void(IntList&, LD, int)> PROB_operator);
};

#endif