/*
 * Implementation of Tarjan's algorithm - Environment class & implementation of Tarjan alg.
 * Part of the Advanced Compilers & Architectures course / "Groot seminarium" 2007
 * Adapted for use in PNTools.
 *
 * Author:        Sven v. Haastregt
 * Last modified: 2011-01-07
 */

#ifndef _TARJAN_H_
#define _TARJAN_H_

#include <vector>
#include <stack>

#include "ppn.h"

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
    
    int id;
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
    ppn::PPNgraphSCCs runTarjansAlgorithm(ppn::PPN *ppn);

  private:
    // Read the input file:
    void importPPN();

    // The actual recursive algorithm
    void tarjansAlgorithm(Node *v, ppn::PPNgraphSCCs &foundSCCs);

    // Cleanup
    void cleanup();

    // Algorithm data:
    int nNodes;
    int nEdges;

    // The graph:
    vector<Edge*> edges;
    vector<Node*> nodes;

    // Pointer to original PPN
    ppn::PPN *ppnref;

    // Tarjan state vars:
    int max_dfs;
    stack<Node*> S;
};

}

#endif
