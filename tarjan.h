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
 *      24-02-12    :   updated for ADG data structure (Teddy Zhai).
 */

#ifndef _TARJAN_H_
#define _TARJAN_H_

#include <vector>
#include <stack>

#include "isl/id.h"

#include "ADG_helper.h"

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

class TarjanClass {
  public:
    // Constructor:
    TarjanClass();
  
    // Destructor:
    ~TarjanClass();
    
    // Returns SCCs w/ size >= 2 using Tarjan's algorithm:
    adg_helper::ADGgraphSCCs runTarjansAlgorithm(adg_helper::ADG_helper*);

  private:
    // Obsolete: Read the input file:
    void importPPN();

    // get Tarjan node from adg node by isl_id
    Node* getTarjanNode(isl_id*);

    // get adg node from Tarjan node by isl_id
    adg_node* getADGNode(isl_id*);

    // The actual recursive algorithm
    void tarjansAlgorithm(Node *v, adg_helper::ADGgraphSCCs &foundSCCs);

    // Cleanup
    void cleanup();

    // Algorithm data:
    int nNodes;
    int nEdges;

    // The graph:
    vector<Edge*> edges;
    vector<Node*> nodes;

    // Pointer to original ADG
    adg_helper::ADG_helper *adgRef;

    // Tarjan state vars:
    int max_dfs;
    stack<Node*> S;
};

}

#endif
