/*
 * Graph algorithms
 */

#include<bits/stdc++.h>
#include<cstdio>
#include<unistd.h>
#include"graph.h"
#include"utils/logging.h"
#include"utils/randomness.h"
#include"utils/ds.h"
using namespace std;

Graph::Graph(int nn, string edgelistmode, string edge_list_file, LL me) {
	n=nn;
	MAX_EDGES=me;
	if (edgelistmode.compare("local-plaintxt")==0) edge_list=new Edge_list_local_plaintxt(edge_list_file);
	if (edgelistmode.compare("local-binary")==0) edge_list=new Edge_list_local_binary(edge_list_file);
}

// Compute degrees in the current induced subgraph on 'vertices',
// by a single pass over the edge list and increment both endpoints if both are still "alive".
void Graph::get_degree_G(FastSet &vertices, IntList &degree) {
	if (vertices.size==0) return;
	fill(degree.begin(),degree.end(),0);
	for(Edge e=edge_list->initial_get_edge(); e.x!=-1; e=edge_list->next_edge())
		if (vertices.contain(e.x) && vertices.contain(e.y))
			degree[e.x]++, degree[e.y]++;
}

// Compute degrees in the groups in H,
// by a single pass over the edge list and increment both endpoints if both are in H and in the same group.
IntList Graph::get_degree_H(IntMap &H) {
	IntList degree(n+1);
	for(Edge e=edge_list->initial_get_edge(); e.x!=-1; e=edge_list->next_edge())
		if (H.count(e.x) && H.count(e.y) && H[e.x]==H[e.y])
			degree[e.x]++, degree[e.y]++;
	return degree;
}

// PEEL subroutine used in ApproxDD
void Graph::PEEL_approx(FastSet &vertices, IntList &degree_G, double epsilon, double alpha, int max_degree) {
	double beta=2/epsilon+2+epsilon;
	int l=ceil(2*(1+beta));
	double threshold_lb=max_degree/(1+alpha), threshold_ub=max_degree/(1+beta);
	// mark vertices with high degree.
	IntMap H;
	for(int x: vertices.lst) if (degree_G[x]>=threshold_lb) H[x]=0;
	while (!H.empty()) {
		// SHAVE
		// random partition to l groups
		for(auto &p: H) p.second=ran_int<int>(0,l-1);
		IntList degree_H=get_degree_H(H);
		vector<IntList> group(l);
		// find vertices with high global degree and low group degree
		for(auto &p: H) if (degree_H[p.first]<=threshold_ub) group[p.second].push_back(p.first);
		// try to remove above vertices, group by group
		for(int i=0; i<l; i++) {
			bool removed_any=0;
			for(int x:group[i]) {
				H.erase(x);
				if (degree_G[x]>=threshold_lb) {
					DD_order.push_back(x);
					vertices.remove(x);
					removed_any=1;
				}
			}
			if (removed_any) get_degree_G(vertices, degree_G);
		}
	}
}

// Remove all vertices by picking the one with highest degree each time,
// by maintaining the vertices and degrees using a heap.
// ** Here the edges of all vertices will be stored in RAM.
void Graph::PEEL_bruteforce(FastSet &vertices, function<bool(int)> is_large, IntList &degree_G, double threshold_lb) {
	// get edges of vertices
	vector<IntList> edges(n+1);
	for(Edge e=edge_list->initial_get_edge(); e.x!=-1; e=edge_list->next_edge())
		if (vertices.contain(e.x) && vertices.contain(e.y) && is_large(e.x) && is_large(e.y)) {
			edges[e.x].push_back(e.y);
			edges[e.y].push_back(e.x);
		}
	// remove vertices by picking the one with highest degree each time
	priority_queue<pair<int,int>> Q;
	for(int x: vertices.lst) if (is_large(x)) Q.push(make_pair(degree_G[x],x));
	while (!Q.empty() && Q.top().first>=threshold_lb) {
		int cur=Q.top().second, curdg=Q.top().first;
		Q.pop();
		if (curdg!=degree_G[cur]) {
			Q.push(make_pair(degree_G[cur],cur));
			continue;
		}
		DD_order.push_back(cur);
		vertices.remove(cur);
		for(int go: edges[cur]) degree_G[go]--;
	}
}

// Select-Large subroutine used in ApproxDD-ES
// Remove high-degree vertices (i.e., sampled degree >= threshold * sp).
// Special case: target degree threshold<1:
// remaining vertices can be appended in arbitrary order (all are effectively "small").
void Graph::select_large(FastSet &vertices, unordered_map<int,IntList> &edges, double threshold, double sp) {
	if (threshold<1) {
		logging(DEBUG,"*** (threshold<1)");
		DD_order.insert(DD_order.end(),vertices.lst.begin(),vertices.lst.end());
		vertices.clear();
	} else {
		for(auto &p: edges) if (vertices.contain(p.first) && count_if(p.second.begin(), p.second.end(), [&](int x){return vertices.contain(x);})>=threshold*sp) {
			DD_order.push_back(p.first);
			vertices.remove(p.first);
		}
	}
}

// Remove isolated vertices in the current induced subgraph.
void Graph::remove_isolated_vertices(FastSet &vertices, IntList &vertices_isolated, IntList &degree) {
	int num_isolated=vertices_isolated.size();
	for(int x: vertices.lst) if (degree[x]==0) vertices_isolated.push_back(x);
	for(int i=num_isolated; i<vertices_isolated.size(); i++) vertices.remove(vertices_isolated[i]);
	logging(DEBUG,"*** Remove "+to_string(vertices_isolated.size()-num_isolated)+" isolated vertices");
}

// Compute an approximate DD ordering using selected modes.
void Graph::compute_DD_order(double epsilon, string mode, double c) {
	logging(INFO,"Computing approx DD-ordering...");
	
	FastSet vertices(n);
	DD_order.clear();
	DD_order.reserve(n);
	IntList vertices_isolated;
	IntList degree_G(n+1);
	get_degree_G(vertices, degree_G);
	double alpha=epsilon/2;						// approximation slack used by ApproxDD peeling.
	double epsilon_hat=epsilon/(4+3*epsilon);	// convenience constant in sampling bounds.
	double T=log(n)/log(1+epsilon/2);			// number of degree scales (geometric decay).
	int g=max((int)(c*T),1);					// number of sampled graphs "per pass block".
	
	while (vertices.size>0) {
		int max_degree=*max_element(degree_G.begin(),degree_G.end());
		
		// (optional) heuristic: if a large-degree prefix fits in memory, peel it exactly.
		if (mode.find("heuristic")!=string::npos) {
			// compute the prefix of vertices and degree that heuristic can reduce
			auto cmp=[&](int x,int y) {return degree_G[x]>degree_G[y] || degree_G[x]==degree_G[y] && x<y;};
			sort(vertices.lst.begin(), vertices.lst.end(), cmp);
			vertices.rebuild_pos();
			IntList degree_dvGRv(vertices.size);	// d(v|GR(v)). GR(v): {u | u<=v}
			for(Edge e=edge_list->initial_get_edge(); e.x!=-1; e=edge_list->next_edge())
				if (vertices.contain(e.x) && vertices.contain(e.y))
					degree_dvGRv[vertices.pos[cmp(e.x,e.y) ?e.y :e.x]]++;
			LL prefix_sum_degree=0;
			int prefix_len=find_if(degree_dvGRv.begin(), degree_dvGRv.end(), [&](int d){
				prefix_sum_degree+=d;
				return prefix_sum_degree>MAX_EDGES;
			}) - degree_dvGRv.begin();
			int delimiter=(prefix_len>=vertices.size ?0 :vertices.lst[prefix_len] );
			// decide if we run heuristic
			double other_approach_bound=(mode.find("approx")!=string::npos ?max_degree/(1+alpha) :max_degree/pow(1+epsilon/2,g) );
			if (degree_G[delimiter]<=other_approach_bound) {
				logging(DEBUG,"** PEEL_heuristic for "+to_string(vertices.size)+" vertices, max degree "+to_string(max_degree)+", select "+to_string(prefix_len)+" vertices");
				PEEL_bruteforce(vertices, [&](int x){return cmp(x,delimiter);}, degree_G, degree_G[delimiter]/(1+epsilon));
				get_degree_G(vertices, degree_G);
				remove_isolated_vertices(vertices, vertices_isolated, degree_G);
				continue;
			}
		}
		
		// ApproxDD
		if (mode.find("approx")!=string::npos) {
			logging(DEBUG,"** PEEL_approx for "+to_string(vertices.size)+" vertices, max degree "+to_string(max_degree));
			PEEL_approx(vertices, degree_G, epsilon, alpha, max_degree);
		}
		
		// ApproxDD-ES
		if (mode.find("ES")!=string::npos) {
			// compute parameters for edge sampling
			vector<double> Delta(g), p(g);
			for(int i=0; i<g; i++) Delta[i]=(i==0 ?max_degree :Delta[i-1]/(1+epsilon/2) );
			double large_degree=Delta[g-1]/(1+epsilon/2);
			auto is_large=[&](int x){return degree_G[x]>=large_degree;};
			int num_large=count_if(vertices.lst.begin(), vertices.lst.end(), is_large);
			for(int i=0; i<g; i++) p[i]=min(min(3*(1+epsilon)*log(100*T*num_large)/(epsilon_hat*epsilon_hat*Delta[i]), (double)MAX_EDGES/(num_large*(LL)g*Delta[i])), 1.0);
			
			if (p[g-1]+(1e-6)>=1) {
				// p=1, we can be more precise, therefore use PEEL_bruteforce
				logging(DEBUG,"** select_large_(p=1) for "+to_string(vertices.size)+" vertices ("+to_string(num_large)+" large), max degree "+to_string(max_degree)+", g "+to_string(g)+", p: "+vec2str(p));
				PEEL_bruteforce(vertices, is_large, degree_G, large_degree/(1+epsilon));
			} else {
				// sample edges for G[0]...G[g-1]
				logging(DEBUG,"*** edge sampling for "+to_string(vertices.size)+" vertices ("+to_string(num_large)+" large), max degree "+to_string(max_degree)+", g "+to_string(g)+", p: "+vec2str(p));
				vector<unordered_map<int,IntList>> G(g);
				LL cnt_edges=0;
				for(Edge e=edge_list->initial_get_edge(); e.x!=-1; e=edge_list->next_edge()) if (vertices.contain(e.x) && vertices.contain(e.y)) {
					if (is_large(e.x))
						for(int i=0; i<g; i++) if (ran_real(0.0,1.0)<=p[i]) {
							G[i][e.x].push_back(e.y);
							cnt_edges++;
						}
					if (is_large(e.y))
						for(int i=0; i<g; i++) if (ran_real(0.0,1.0)<=p[i]) {
							G[i][e.y].push_back(e.x);
							cnt_edges++;
						}
				}
				// select_large
				logging(DEBUG,"** select_large for "+to_string(vertices.size)+" vertices ("+to_string(num_large)+" large), max degree "+to_string(max_degree)+", edges "+to_string(cnt_edges));
				for(int i=0; i<g; i++) if (vertices.size>0) select_large(vertices, G[i], Delta[i]/(1+3*epsilon/4), p[i]);
			}
			get_degree_G(vertices, degree_G);
		}
		
		remove_isolated_vertices(vertices, vertices_isolated, degree_G);
	}
	
	DD_order.insert(DD_order.end(), vertices_isolated.begin(), vertices_isolated.end());
}

// -----------------------------------------------------------------------------
// Disjoint Set Union (Union-Find) used by compute_distrib().
//
// compute_distrib() needs to decide, for each vertex v, whether the "bucket"
// N(v) is non-empty, i.e., whether G(v) contains at least one connected
// (k-1)-vertex extension reachable from v by RAND-GROW.
// We cannot materialize all of G(v), so we use a DSU per v over its "heavy"
// neighbors and store only up to (k-2) "light" edges per component. This is
// enough to detect the existence of a (k-1)-clique-like growth structure needed
// to ensure N(v)>0.
class DSU {
	public:
	struct DSU_element {
		int root, size, last_heavy;
	};
	int n;
	vector<DSU_element> a;
	
	DSU(int nn, IntList &last_heavy) {
		n=nn;
		a.resize(n+1);
		for(int i=1; i<=n; i++) a[i]=(DSU_element){i,1,last_heavy[i]};
	}
	
	int get_root(int x) {return (a[x].root==x) ?x :a[x].root=get_root(a[x].root) ;}
	
	void merge(int x,int y) {
		x=get_root(x), y=get_root(y);
		if (x!=y) {
			a[y].root=x;
			a[x].size+=a[y].size;
			a[x].last_heavy=max(a[x].last_heavy,a[y].last_heavy);
		}
	}
};

LL fac(int k) {
	LL re=1;
	for(; k; k--) re*=k;
	return re;
}
// -----------------------------------------------------------------------------

// Initial distribution
// Principles:
//   1. If v has degree at least k-1 in G(v), v is heavy;
//   2. If v connects to more than k-1 vertices in G(v), v is heavy;
//   3. If v connects to a heavy vertex u, v becomes heavy in G(w) for w<=u.
void Graph::compute_distrib(int k, double epsilon) {
	logging(INFO,"Compute distribution...");
	
	// for each v, compute:
	// d(v|G(v)), light edge list in G(v), and the last (i.e., with largest DD rank) heavy vertex in G(v).
	// (Here we only consider heavy vertices of principle 1.)
	IntList degree_dvGv(n+1), last_heavy(n+1);
	vector<int> light_edge_list((n+1)*(k-2));
	for(Edge e=edge_list->initial_get_edge(); e.x!=-1; e=edge_list->next_edge()) {
		if (DD_ranking[e.x]>DD_ranking[e.y]) swap(e.x,e.y);
		if (++degree_dvGv[e.x]<=k-2) {
			// e.x is still light
			light_edge_list[e.x*(k-2)+degree_dvGv[e.x]-1]=e.y;
		} else {
			// e.x becomes heavy
			last_heavy[e.x]=DD_ranking[e.x];
			// if it is the first time e.x becomes heavy, then update last_heavy of its neighbors
			if (degree_dvGv[e.x]==k-1)
				for(int zi=0; zi<k-2; zi++) {
					int z=light_edge_list[e.x*(k-2)+zi];
					last_heavy[z]=max(last_heavy[z], DD_ranking[e.x]);
				}
			last_heavy[e.y]=max(last_heavy[e.y], DD_ranking[e.x]);
		}
	}
	
	// compute pr[v] <- d(v|G(v))^{k-1} if the bucket of v (i.e., k-graphlets in G(v) containing v) is not empty, otherwise 0
	// (Here we deal with principles 2 and 3.)
	pr.resize(n+1);
	DSU dsu(n,last_heavy);
	for(int i=n-1; i>=0; i--) {
		int x=DD_order[i];
		if (degree_dvGv[x]<=k-2) {
			for(int y=0; y<degree_dvGv[x]; y++) dsu.merge(x, light_edge_list[x*(k-2)+y]);
			if (dsu.a[x].size>=k || dsu.a[x].last_heavy>=i) pr[x]=pow((LD)degree_dvGv[x],k-1);
		} else {
			pr[x]=pow((LD)degree_dvGv[x],k-1);
		}
	}
	
	// normalize pr and compute Gamma
	LD Z=reduce(pr.begin(), pr.end());
	for(int i=1; i<=n; i++) pr[i]/=Z;
	Gamma=1/(fac(k-1)*Z*pow(1+epsilon,k-1));
	// logging(DEBUG, "*** max(p) = "+to_string((double)*max_element(pr.begin(),pr.end())));
}

// Preprocessing.
// Run only once.
void Graph::sampling_preprocess(int k, double epsilon, IntList &provided_DD_order, string DD_order_mode, double c) {
	if (provided_DD_order.empty()) {
		compute_DD_order(epsilon, DD_order_mode, c);
		logging(INFO,"Number of passes for DD-ordering = "+to_string(edge_list->passes));
	} else {
		DD_order=provided_DD_order;
	}
	DD_ranking.resize(n+1);
	for(int i=0; i<n; i++) DD_ranking[DD_order[i]]=i;
	compute_distrib(k, epsilon);
}

// A degree / neighbor-count query used by RAND_GROW and PROB.
// What we want: d(u | G(v))  = number of neighbors of u that are >= v in the order.
// We batch many queries together, sort them by threshold, and answer all of them in ONE edge pass (see get_degree_duGv()).
struct Query {
	int u;
	IntList v_list,cnt,queries,num_index;
	vector<vector<int>> queries_for_v;
	
	Query(int uu=0) {u=uu;}
	
	void build_cnt() {
		sort(v_list.begin(), v_list.end());
		v_list.resize(unique(v_list.begin(),v_list.end())-v_list.begin());
		cnt.resize(v_list.size()+1);
	}
	
	void build_queries_for_v(unordered_map<ULL,int> &degree_duGv, IntList &DD_order) {
		num_index.resize(v_list.size()+1);
		for(int i=0; i<v_list.size(); i++) num_index[i]=degree_duGv[Edge::hash(DD_order[v_list[i]],u)];
		sort(queries.begin(),queries.end(),greater<int>());
		queries.resize(unique(queries.begin(),queries.end())-queries.begin());
		queries_for_v.resize(v_list.size());
		int vi=0;
		for(int q: queries) {
			while (vi+1<v_list.size() && q<=num_index[vi+1]) vi++;
			queries_for_v[vi].push_back(q);
		}
		queries.clear();
	}
};

tuple<unordered_map<ULL,int>,IntList> Graph::get_degree_duGv(vector<IntList> &samples, int sample_size) {
	unordered_map<int,Query> L;
	unordered_set<ULL> edges_internal_queries;
	// set up queries
	for(IntList &sample: samples)
		for(int x: sample) {
			for(int y: sample) if (x<y) edges_internal_queries.insert(Edge::hash(x,y));
			if (!L.count(x)) L[x]=Query(x);
			L[x].v_list.push_back(DD_ranking[sample[0]]);
		}
	// move queries to appropriate v
	for(auto &p: L) p.second.build_cnt();
	// answer queries and obtain the internal edge set
	for(Edge e=edge_list->initial_get_edge(); e.x!=-1; e=edge_list->next_edge()) {
		if (L.count(e.x)) L[e.x].cnt[upper_bound(L[e.x].v_list.begin(),L[e.x].v_list.end(),DD_ranking[e.y])-L[e.x].v_list.begin()]++;
		if (L.count(e.y)) L[e.y].cnt[upper_bound(L[e.y].v_list.begin(),L[e.y].v_list.end(),DD_ranking[e.x])-L[e.y].v_list.begin()]++;
		edges_internal_queries.erase(e.hash());
	}
	// obtain degree
	unordered_map<ULL,int> degree;
	for(auto &p: L)
		for(int l=(int)(p.second.cnt.size())-1; l; l--) {
			degree[Edge::hash(DD_order[p.second.v_list[l-1]],p.first)]=p.second.cnt[l];
			p.second.cnt[l-1]+=p.second.cnt[l];
		}
	// obtain edges_internal
	// for each sample, the internal edges can be represented by k(k-1)/2 bits: whether (s[0],s[1]),(s[0],s[2])...,(s[k-2],s[k-1]) exist
	IntList edges_internal(samples.size());
	for(int i=0; i<samples.size(); i++) {
		for(int x=0; x<sample_size; x++)
			for(int y=x+1; y<sample_size; y++)
				(edges_internal[i]<<=1)|=(!edges_internal_queries.count(Edge(samples[i][x],samples[i][y]).hash()));
	}
	return {degree, edges_internal};
}

int is_internal_edge(int edges_internal, int x, int y, int len) {
	if (x==y) return 0;
	return (x<y) ?((edges_internal>>((((len-x-2)*(len-x-1))>>1)+len-y-1))&1)
				 :((edges_internal>>((((len-y-2)*(len-y-1))>>1)+len-x-1))&1);
}

vector<IntList> Graph::RAND_GROW_fast(IntList &v, int k, int batch_size) {
	vector<IntList> samples(batch_size);
	for(int j=0; j<batch_size; j++) samples[j].push_back(v[j]);
	for(int i=1; i<k; i++) {
		auto [degree_duGv, edges_internal]=get_degree_duGv(samples, i);
		// set up queries
		unordered_map<int,Query> L;
		vector<IntList> original_query(batch_size);
		unordered_map<ULL,int> answers;
		IntList up(batch_size);
		for(int j=0; j<batch_size; j++) {
			IntList degree_duS(i), weight(i);
			for(int l=0; l<i; l++) {
				int x=samples[j][l];
				for(int t=0; t<i; t++) degree_duS[l]+=is_internal_edge(edges_internal[j], l, t, i);
				weight[l]=degree_duGv[Edge::hash(v[j],x)]-degree_duS[l];
			}
			int uid=ran_discrete(weight), u=up[j]=samples[j][uid];
			original_query[j] = ran_int_range(1, degree_duGv[Edge::hash(v[j],u)], degree_duS[uid]+1);
			if (!L.count(u)) L[u]=Query(u);
			L[u].v_list.push_back(DD_ranking[v[j]]);
			L[u].queries.insert(L[u].queries.end(), original_query[j].begin(), original_query[j].end());
		}
		// move queries to appropriate v
		for(auto &p:L) {
			p.second.build_cnt();
			p.second.build_queries_for_v(degree_duGv,DD_order);
		}
		// answer queries
		for(Edge ee=edge_list->initial_get_edge(); ee.x!=-1; ee=edge_list->next_edge())
			for(Edge e: {ee, ee.reverse()}) if (L.count(e.x)) {
				Query &Lex=L[e.x];
				int it=upper_bound(Lex.v_list.begin(),Lex.v_list.end(),DD_ranking[e.y])-Lex.v_list.begin();
				if (!it) continue;
				Lex.cnt[it]++;
				for(auto &q=Lex.queries_for_v[it-1]; !q.empty() && q.back()==Lex.cnt[it]+Lex.num_index[it]; q.pop_back()) answers[Edge::hash(e.x,q.back())]=e.y;
			}
		// sample by answers
		for(int j=0; j<batch_size; j++)
			for(int eid:original_query[j]) {
				int u=answers[Edge::hash(up[j],eid)];
				if (find(samples[j].begin(),samples[j].end(),u)==samples[j].end()) {
					samples[j].push_back(u);
					break;
				}
			}
	}
	return samples;
}

void Graph::PROB(vector<IntList> &samples, int k, function<void(IntList&, LD, int)> PROB_operator) {
	auto [degree_duGv, edges_internal]=get_degree_duGv(samples, k);
	int K=(1<<(k-1));
	for(int j=0; j<samples.size(); j++) {
		IntList &sample=samples[j];
		auto is_j_internal_edge=[&](int x,int y) {return is_internal_edge(edges_internal[j], x, y, k);};
		// compute the probability of samples[j], storing it in q[K-1]
		vector<LD> q(K);
		q[0]=1;
		for(int S=0; S<K; S++) {
			IntList Sset(1,0);
			for(int x=0; x<k-1; x++) if ((S>>x)&1) Sset.push_back(x+1);
			LL ci=0;
			for(int x:Sset) ci+=degree_duGv[Edge::hash(sample[0],sample[x])]-count_if(Sset.begin(), Sset.end(), [&](int y){return is_j_internal_edge(x,y);});
			for(int x=0; x<k-1; x++) if (!((S>>x)&1)) {
				int ni=count_if(Sset.begin(), Sset.end(), [&](int y){return is_j_internal_edge(x+1,y);});
				q[S|(1<<x)]+=q[S]*ni/ci;
			}
		}
		
		PROB_operator(sample, pr[sample[0]]*q[K-1], edges_internal[j]);
	}
}

void Graph::sample_one_batch(int k, int batch_size, function<void(IntList&, LD, int)> PROB_operator) {
	IntList v=ran_discrete(pr, batch_size);
	vector<IntList> samples=RAND_GROW_fast(v, k, batch_size);
	PROB(samples, k, PROB_operator);
}

vector<IntList> Graph::sample_ugs(int k, int target) {
	// For uniform graphlet sampling. A graphlet is represented by a sequence of vertices.
	// Sample batch_size trials once a batch until the number of successful samples reaches target.
	int batch_size=MAX_EDGES/(k*k);
	logging(INFO,"Start sampling with batch size "+to_string(batch_size)+" ...");
	vector<IntList> samples;
	auto update_samples=[&](IntList &sample, LD p, int adjacency) {
		if (ran_real(0.0,1.0)*p<=Gamma) samples.push_back(sample);
	};
	for(int batch_count=1; samples.size()<target; batch_count++) {
		sample_one_batch(k, batch_size, update_samples);
		if (samples.size()>=target) {
			logging(INFO,"Sampled "+to_string(samples.size())+" graphlets successfully after "+to_string(batch_count*batch_size)+" trails");
			samples.resize(target);
			return samples;
		}
		logging(INFO,"The "+to_string(batch_count)+"-th batch. Totally "+to_string(batch_count*batch_size)+" trials, "+to_string(samples.size())+" successful trials.");
	}
	return samples;
}

vector<LD> Graph::sample_gd(int k, int target, int graphlet_num, vector<int> &mapping, bool is_rejection) {
	// For computing graphlet distribution, representing a graphlet by an adjacency matrix.
	int batch_size=MAX_EDGES/(k*k);
	logging(INFO,"Start sampling with batch size "+to_string(batch_size)+" ...");
	vector<LD> counter(graphlet_num);
	function<void(IntList&, LD, int)> update_counter;
	if (is_rejection) {
		update_counter=[&](IntList &sample, LD p, int adjacency) {
			counter[mapping[adjacency]]+=(ran_real(0.0,1.0)*p<=Gamma);
		};
	} else {
		update_counter=[&](IntList &sample, LD p, int adjacency) {
			counter[mapping[adjacency]]+=1/p;
		};
	}
	for(int batch_count=1, sample_num=0; sample_num<target; batch_count++, sample_num+=batch_size) {
		sample_one_batch(k, batch_size, update_counter);
		logging(INFO,"The "+to_string(batch_count)+"-th batch. Totally "+to_string(batch_count*batch_size)+" trials.");
	}
	
	// normalize counters
	logging(INFO,"Normalizing counters...");
	LD sum=reduce(counter.begin(), counter.end());
	for(LD &c: counter) c/=sum;
	return counter;
}

const int MAX_BUCKET_SIZE=1e8;

vector<LD> Graph::evaluate_DD_order() {
	logging(INFO,"Evaluating DD order...");
	
	// sort edges according to the DD order, using external bucket sorting.
	// compute d(v|G(v))
	DD_ranking.resize(n+1);
	for(int i=0; i<n; i++) DD_ranking[DD_order[i]]=i;
	vector<int> dvGv(n+1);
	for(Edge e=edge_list->initial_get_edge(); e.x!=-1; e=edge_list->next_edge())
		dvGv[DD_ranking[e.x]<DD_ranking[e.y] ?e.x :e.y ]++;
	// assign vertices to buckets. Each bucket contains at most MAX_BUCKET_SIZE edges.
	vector<int> bucket_size(1);
	vector<int> bucket_id(n+1);
	for(int i=0; i<n; i++) {
		int x=DD_order[i];
		if (bucket_size.back()+dvGv[x]>MAX_BUCKET_SIZE) bucket_size.push_back(dvGv[x]);
			else bucket_size.back()+=dvGv[x];
		bucket_id[x]=bucket_size.size()-1;
	}
	dvGv.clear();
	logging(DEBUG,"Num of buckets: "+to_string(bucket_size.size()));
	// distribute edges to buckets
	string bucket_file[bucket_size.size()];
	ofstream fbucket_w[bucket_size.size()];
	pid_t pid=getpid();
	for(int i=0; i<bucket_size.size(); i++) {
		bucket_file[i]=to_string(pid)+"temp"+to_string(i)+"_"+edge_list->edge_list_file.substr(0,edge_list->edge_list_file.rfind('.'))+".bin";
		fbucket_w[i].open(bucket_file[i], ios::binary);
	}
	for(Edge e=edge_list->initial_get_edge(); e.x!=-1; e=edge_list->next_edge()) {
		if (DD_ranking[e.x]>DD_ranking[e.y]) swap(e.x, e.y);
		fbucket_w[bucket_id[e.x]].write(reinterpret_cast<char*>(&e), sizeof(Edge));
	}
	for(int i=0; i<bucket_size.size(); i++) fbucket_w[i].close();
	bucket_id.clear();
	
	// process edges in the reverse order, and evaluate the DD order
	vector<int> duGv(n+1);
	int max_degree=0;
	vector<LD> theta(n+1);
	int cur_bucket=bucket_size.size();
	vector<Edge> cur_bucket_edges;
	Edge e;
	auto get_next_edge=[&]() {
		while (cur_bucket_edges.empty() && cur_bucket>=0) {
			do {
				cur_bucket--;
			} while (bucket_size[cur_bucket]==0 && cur_bucket>=0);
			if (cur_bucket<0) break;
			cur_bucket_edges.resize(bucket_size[cur_bucket]);
			ifstream fbucket_r(bucket_file[cur_bucket]);
			fbucket_r.read(reinterpret_cast<char*>(cur_bucket_edges.data()), sizeof(Edge)*bucket_size[cur_bucket]);
			sort(cur_bucket_edges.begin(), cur_bucket_edges.end(), [&](const Edge &a,const Edge &b){
				return DD_ranking[a.x]<DD_ranking[b.x] || DD_ranking[a.x]==DD_ranking[b.x] && DD_ranking[a.y]<DD_ranking[b.y];
			});
			fbucket_r.close();
			remove(bucket_file[cur_bucket].c_str());
		}
		if (cur_bucket<0) {
			e=Edge(-1,-1);
		} else {
			e=cur_bucket_edges.back();
			cur_bucket_edges.pop_back();
		}
	};
	get_next_edge();
	for(int i=n-1; i>=0; i--) {
		while (e.x!=-1 && DD_ranking[e.x]>=i) {
			duGv[e.x]++, duGv[e.y]++;
			max_degree=max(max_degree,max(duGv[e.x],duGv[e.y]));
			get_next_edge();
		}
		theta[DD_order[i]]=(duGv[DD_order[i]] ?(LD)duGv[DD_order[i]]/max_degree :1 );
	}
	
	return theta;
}

Graph::~Graph() {
	delete edge_list;
}

// *********** the following is just for experiments

void Graph::sample_gd_both(int k, int target, int graphlet_num, vector<int> &mapping, vector<int> &graphlets) {
	int batch_size=MAX_EDGES/(k*k);
	logging(INFO,"Start sampling with batch size "+to_string(batch_size)+" ...");
	vector<LD> counter_gdrejection(graphlet_num), counter_gdcounter(graphlet_num);
	auto update_counter=[&](IntList &sample, LD p, int adjacency) {
		counter_gdrejection[mapping[adjacency]]+=(ran_real(0.0,1.0)*p<=Gamma);
		counter_gdcounter[mapping[adjacency]]+=1/p;
	};
	
	for(int batch_count=1, sample_num=0; sample_num<target; batch_count++, sample_num+=batch_size) {
		sample_one_batch(k, batch_size, update_counter);
		logging(INFO,"The "+to_string(batch_count)+"-th batch. Totally "+to_string(batch_count*batch_size)+" trials.");
		
		LD sum=reduce(counter_gdrejection.begin(), counter_gdrejection.end());
		for(int i=0; i<graphlets.size(); i++) logging(INFO, "GDRejection "+to_string(batch_count)+" "+to_string(graphlets[i])+" "+to_string(counter_gdrejection[i]/sum));
		logging(INFO, "ugs "+to_string(batch_count)+" successful trials "+to_string(sum));
		sum=reduce(counter_gdcounter.begin(), counter_gdcounter.end());
		for(int i=0; i<graphlets.size(); i++) logging(INFO, "GDCounter "+to_string(batch_count)+" "+to_string(graphlets[i])+" "+to_string(counter_gdcounter[i]/sum));
	}
}