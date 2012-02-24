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

#include "tarjan.h"

using namespace Tarjan;

//#define MIN(x,y) ((x > y) ? y:x)
static inline int minval(int x, int y) {
  return (x > y) ? y : x;
}

Edge::Edge() {
  from = NULL;
  to = NULL;
  visited = false;
}


Node::Node() {
  dfs = 0;
  lowlink = 0;
  visited = false;
  onstack = false;
}


// Dump the minus set:
void Node::DumpMinusSet() {
  printf("Minusset of node %s: ", isl_id_get_name(id));
  for (unsigned int i = 0; i < minusset.size(); i++) {
    printf("%s%s ", isl_id_get_name(minusset[i]->id), i == minusset.size()-1 ? "" : ",");
  }
  printf("\n");
}


// Dump the associations made to this node:
void Node::DumpAssociations() {
  printf("Associations of root node %s: ", isl_id_get_name(id));
  for (unsigned int i = 0; i < associations.size(); i++) {
    printf("%s%s ", isl_id_get_name(associations[i]->id), i == associations.size()-1 ? "" : ",");
  }
  printf("\n");
}


// Is the minusset of this node a subset of the minusset of w?
// Thus: does ( this- C w- ) hold?
bool Node::isMinusSetSubsetOf(Node *w) {
  for (unsigned int vi = 0; vi < minusset.size(); vi++) {
    for (unsigned int wi = 0; wi < w->minusset.size(); wi++) {
      if (minusset[vi] == w->minusset[wi]) {
        // Found it, try next element
        break;
      }
      else {
        if (wi == w->minusset.size()-1) {
          // Last iteration, and we didn't find this element
          return false;
        }
      }
    }
  }
  // All done, all found
  return true;
}


// Is w element of this node's minusset?
// Thus: does ( w elem. of this- ) hold?
bool Node::isElemOf(Node *w) {
  for (unsigned int i = 0; i < minusset.size(); i++) {
    if (w == minusset[i]) {
      return true;
    }
  }
  return false;
}


// Computes (this U v) and assigns the result to this
void Node::AssignMinusSetUnion(Node *v) {
  for (unsigned int vi = 0; vi < v->minusset.size(); vi++) {
    for (unsigned int wi = 0; wi < minusset.size(); wi++) {
      if (v->minusset[vi] == minusset[wi]) {
        // Found it already, try next element
        break;
      }
      else {
        if (wi == minusset.size()-1) {
          // Last iteration, and we didn't found this element, so add it
          minusset.push_back(v->minusset[vi]);
        }
      }
    }
  }
  
}


// Associate v to this; also copy v's associations
void Node::Associate(Node *v) {
  for (unsigned int i = 0; i < v->associations.size(); i++) {
    associations.push_back(v->associations[i]);
  }
  associations.push_back(v);
  
}


// Constructor
TarjanClass::TarjanClass() {
  nNodes = 0;
  nEdges = 0;
}



// Destructor
TarjanClass::~TarjanClass() {
  cleanup();
}


void TarjanClass::cleanup() {
  nNodes = 0;
  nEdges = 0;
  while (!S.empty()) {
    S.pop();
  }
  while (!edges.empty()) {
    delete edges.back();
    edges.pop_back();
  }
  while (!nodes.empty()) {
    delete nodes.back();
    nodes.pop_back();
  }
}



// Read the input file
void TarjanClass::importPPN() {
  Edge *edge;
  Node *node;

  this->nNodes = this->adgRef->getNodes().size();
  this->nEdges = this->adgRef->getEdges().size();
  
  // Create nodes
  for (int i = 0; i < nNodes; i++) {
    node = new Node;
    node->id = this->adgRef->getNodes()[i]->name;
    nodes.push_back(node);
  }
  
  // Read edges
  adg_helper::Edges adgEdges = this->adgRef->getEdges();
  for (int i = 0; i < nEdges; i++) {
    edge = new Edge;
    edge->from	= getTarjanNode(adgEdges[i]->from_node_name);
    edge->to 	= 	getTarjanNode(adgEdges[i]->to_node_name);
    edges.push_back(edge);
  }
  
}


// get Tarjan node from adg node by isl_id
Node*
TarjanClass::getTarjanNode(isl_id* name){
	for (int i = 0; i < this->nodes.size(); ++i) {
		if (nodes[i]->id != name) continue;

		return nodes[i];
	}

	// should not reach here
	fprintf(stderr, "Tarjan node with name: %s is not found!!", isl_id_get_name(name));
}

// get adg node from Tarjan node by isl_id
adg_node*
TarjanClass::getADGNode(isl_id* name){
	adg_helper::Nodes nodes = this->adgRef->getNodes();
	for (int i = 0; i < nodes.size(); ++i) {
		if (nodes[i]->name != name) continue;

		return nodes[i];
	}

	// should not reach here
	fprintf(stderr, "ADG node with name: %s is not found!!", isl_id_get_name(name));
}


// The actual (recursive) algorithm
void TarjanClass::tarjansAlgorithm(Node *v, adg_helper::ADGgraphSCCs &foundSCCs) {
  Node *vv;

  v->dfs = max_dfs;
  v->lowlink = max_dfs;
  max_dfs++;
  S.push(v);
  v->onstack = true;
  v->visited = true;
  
  for (unsigned int i = 0; i < edges.size(); i++) {
    if (edges[i]->from == v) {
      vv = edges[i]->to;
      if (vv->visited == false) {
        tarjansAlgorithm(vv, foundSCCs);
        v->lowlink = minval(v->lowlink, vv->lowlink);
      }
      else if (vv->onstack) {
        v->lowlink = minval(v->lowlink, vv->dfs);
      }
    }
  }
  
  if (v->lowlink == v->dfs) {
    //printf("SCC:");
    adg_helper::Nodes scc;
    do {
      vv = S.top();
      S.pop();
      vv->onstack = false;
      //printf(" %d%s", vv->id, (vv == v)?"":",");
      adg_helper::Node *adgNd = getADGNode(vv->id);
      scc.push_back(adgNd);
    } while (vv != v);
    if (scc.size() > 1) {
      // Only report SCCs with at least 2 nodes
      // Note: reporting ALL SCCs is NOT a matter of just removing the if condition
      foundSCCs.push_back(scc);
    }
    //printf("\n");
  }
}


// Execute Tarjan's algorithm; public function
adg_helper::ADGgraphSCCs
TarjanClass::runTarjansAlgorithm(adg_helper::ADG_helper *adgHelper) {
  adg_helper::ADGgraphSCCs foundSCCs;
  max_dfs = 0;
  this->adgRef = adgHelper;
  importPPN();
  tarjansAlgorithm(nodes[0], foundSCCs);
  cleanup();
  return foundSCCs;
}

