/*
 * Computing the list of k-graphlet isomorphism classes, and
 * Build the mapping from k-graphlets to class id
 *
 * Adjacency mask for a connected graph of k vertices:
 *   k*(k-1)/2 bits: whether edge(1,2), ..., edge(1,k), edge(2,3), ..., edge(2,k), ..., edge(k-1,k) exist respectively
 * Adjacency mask for an isomorphic class:
 *   the minimum adjacency mask among the graphs in the class
 */

#ifndef ALLGRAPHLETS
#define ALLGRAPHLETS

#include<bits/stdc++.h>
using namespace std;

tuple<vector<int>,vector<int>> get_all_graphlets(int k);
// Returns:
//   - graphlets: list of representative adjacency masks for each class
//   - mapping: canonical-hash -> class id

#endif