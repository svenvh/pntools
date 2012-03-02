/*
 *	Copyright (c) 2012 Leiden University (LERC group at LIACS).
 * 	All rights reserved.
 *
 * Implementation of Tarjan's algorithm - Environment class & implementation of Tarjan alg.
 * Part of the Advanced Compilers & Architectures course / "Groot seminarium" 2007
 * Adapted for use in PNTools.
 *
 * Author:        Sven v. Haastregt
 *
 *
 * History:
 *      24-02-12    	:   updated for ADG data structure (Teddy Zhai).
 *      2-03-12    	:   made this independent of adg/pdg structure (Teddy Zhai).
 */

#ifndef _TARJAN_H_
#define _TARJAN_H_

#include <vector>
#include <stack>

#include "isl/id.h"

using namespace std;

namespace Tarjan {

class Node {
  public:
    Node();
    void DumpMinusSet();
    void DumpAssociations();
    bool isMinusSetSubsetOf(Node *w);
    bool isElemOf(Node *w);
    void AssignMinusSetUnion(Node *v);
    void Associate(Node *v);
    
    isl_id *id;
    vector<Node*> minusset;
    vector<Node*> associations;
    int dfs;
    int lowlink;
    bool visited;
    bool onstack;
};

class Edge {
  public:
    Edge();
  
    Node *from;
    Node *to;
    bool visited;
};

class Graph {
public:
	std::vector<Node*> nodes;
	std::vector<Edge*> edges;
};

typedef std::vector<std::vector<Node*> > SCCs_t;

class TarjanClass {
  public:
    // Constructor:
    TarjanClass();

	TarjanClass(Graph*);

    // Destructor:
    ~TarjanClass();

    void runTarjansAlgorithm(Node *v, SCCs_t &foundSCCs);
    
    // Cleanup
	void cleanup();

  private:

    // Algorithm data:
    int nNodes;
    int nEdges;

    // The graph:
    vector<Edge*> edges;
    vector<Node*> nodes;

    // Tarjan state vars:
    int max_dfs;
    stack<Node*> S;
};

}

#endif
