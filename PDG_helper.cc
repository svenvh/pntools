/*		Copyright (c) 2012 Leiden University (LERC group at LIACS).
 * 		All rights reserved.
 *
 *		PDG_helper.cc
 *
 *  	Created on: Feb 7, 2012
 *      Author: Teddy Zhai
 */

#include "isa/pdg.h"

#include "tarjan.h"
#include "PDG_helper.h"


using pdg::PDG;
using namespace pdg;

namespace pdg_helper {

PDG_helper::PDG_helper(isl_ctx *ctx, PDG *pdg) {
	_ctx = ctx;
	_pdg = pdg;

	// initialize a map containing name (isl_id) and node (pdg::node)
	pNodes_t nodes = getNodes();
	for (int i = 0; i < nodes.size(); ++i) {
		isl_id *name = getNameISL(nodes[i]);
		_nodesMap[name] = nodes[i];
	}

	/* initialize source and sink nodes */
	getSrcSnkNodes(&(_srcNodes), &(_snkNodes));
	assert(_srcNodes.size() > 0 && _snkNodes.size());

	/* initialize SCCs */
	_SCCs = getSCCs();
}

PDG_helper::~PDG_helper() {
	for (std::map<isl_id*, pNode_t*>::iterator mit = _nodesMap.begin();
			mit != _nodesMap.end();
			++mit)
	{
		isl_id_free(mit->first);
	}
}

pNodes_t
PDG_helper::getNodes(){
	return _pdg->nodes.v;
}

pNode_t*
PDG_helper::getNode(const std::string &name){
	pNodes_t nodes = _pdg->nodes.v;
	for (int i = 0; i < nodes.size(); ++i) {
		if (name == nodes[i]->name->s) {
			return nodes[i];
		}
	}

	// should not reach here
	fprintf(stderr, "ERROR: node with name: %s is not found.", name.c_str());
}

__isl_give isl_id*
PDG_helper::getNameISL(const pNode_t *pdgNode){
	isl_id *name;

	isl_set *domain = pdgNode->source->set;

	name = isl_set_get_tuple_id(domain);

	return name;
}


bool
PDG_helper::isSourceNode(const pNode_t *node){
	for (int i = 0; i < _srcNodes.size(); ++i) {
		if (node->name->s ==_srcNodes[i]->name->s ) {
			return true;
		}
	}

	return false;
}

bool
PDG_helper::isSinkNode(const pNode_t *node){
	for (int i = 0; i < _snkNodes.size(); ++i) {
		if (node->name->s ==_snkNodes[i]->name->s ) {
			return true;
		}
	}

	return false;
}


std::string PDG_helper::getFunctionName(pdg::node const *node) {
	return node->statement->top_function->name->s;
}


void
PDG_helper::getSrcSnkNodes(pNodes_t* srcNodes, pNodes_t* snkNodes){
	std::map<node*, bool> srcNodesMap;
	std::map<node*, bool> snkNodesMap;

	pNodes_t nodes = _pdg->nodes.v;
	for (int i = 0; i < nodes.size(); ++i) {
		srcNodesMap[nodes[i]]= false;
		snkNodesMap[nodes[i]]= false;
	}

	// for each dependence, source node is marked as source
	// sink node is marked with sink
	pDeps_t deps = _pdg->dependences.v;
	for (int i = 0; i < deps.size(); ++i) {
		srcNodesMap[deps[i]->from] = true;
		snkNodesMap[deps[i]->to] = true;
	}

	// if a node is source but not sink of any other nodes, it is a source of the PDG
	// if a node is sink but not source of any other nodes, it is a sink of the PDG
	for (int i = 0; i < nodes.size(); ++i) {
		if (srcNodesMap[nodes[i]] == true && snkNodesMap[nodes[i]] == false) {
			srcNodes->push_back(nodes[i]);
		}
		if (srcNodesMap[nodes[i]] == false && snkNodesMap[nodes[i]] == true) {
			snkNodes->push_back(nodes[i]);
		}
	}
}

pNode_t*
PDG_helper::getSourceNode(const pDep_t *dep){
	pNode_t *rt = NULL;

	isl_map *depMap = isl_map_copy(dep->relation->map);

	// get range of the relation
	isl_set *domMap = isl_map_domain(depMap);

	isl_id *name = isl_set_get_tuple_id(domMap);
	std::string name_str(isl_id_get_name(name));
	rt = getNode(name_str);

	isl_set_free(domMap);
	isl_id_free(name);
	assert(rt != NULL);
	return rt;
}

pNode_t*
PDG_helper::getSnkNode(const pDep_t *dep){
	pNode_t *rt = NULL;

	isl_map *depMap = isl_map_copy(dep->relation->map);

	// get range of the relation
	isl_set *ranMap = isl_map_range(depMap);

	isl_id *name = isl_set_get_tuple_id(ranMap);
	std::string name_str(isl_id_get_name(name));

	rt = getNode(name_str);

	isl_set_free(ranMap);
	isl_id_free(name);
	assert(rt != NULL);
	return rt;
}

pNodes_t
PDG_helper::getDataflowNodes(){
	pNodes_t dataflowNodes;

	std::map<node*, bool> srcNodesMap;
	std::map<node*, bool> snkNodesMap;

	pNodes_t nodes = _pdg->nodes.v;
	for (int i = 0; i < nodes.size(); ++i) {
		srcNodesMap[nodes[i]]= false;
		snkNodesMap[nodes[i]]= false;
	}

	// for each dependence, source node is marked as source
	// sink node is marked with sink
	pDeps_t deps = _pdg->dependences.v;
	for (int i = 0; i < deps.size(); ++i) {
		srcNodesMap[deps[i]->from] = true;
		snkNodesMap[deps[i]->to] = true;
	}

	// if a node is source but not sink of any other nodes, it is a source of the PDG
	// if a node is sink but not source of any other nodes, it is a sink of the PDG
	for (int i = 0; i < nodes.size(); ++i) {
		if (srcNodesMap[nodes[i]] == true && snkNodesMap[nodes[i]] == true) {
			dataflowNodes.push_back(nodes[i]);
		}
	}

	return dataflowNodes;
}

// Returns all edges connecting from -> to.
pDeps_t PDG_helper::getEdges(pdg::node const *from, pdg::node const *to) {
  pDeps_t edges;
	for (int i = 0; i < _pdg->dependences.size(); i++) {
		pdg::dependence *dep = _pdg->dependences[i];
		if (dep->from == from && dep->to == to) {
			edges.push_back(dep);
		}
	}
	return edges;
}

bool
PDG_helper::isChain(){

}

bool
PDG_helper::isTree(){

}

pDeps_t
PDG_helper::getDependences(){
	return _pdg->dependences.v;
}

bool
PDG_helper::isSelfDependence(const pDep_t* dep){
	if (getSourceNode(dep) == getSnkNode(dep)) {
		return true;
	} else {
		return false;
	}
}

PDGgraphSCCs
PDG_helper::getSCCs(){
	PDGgraphSCCs ret;

	std::map<pNode_t*, isl_id*> nodeNameMap;

	/* Initialize tarjan graph using adg */
	// 1. Create nodes
	std::vector<Tarjan::Node*> tarjanNodes;
	pNodes_t pdgNodes = getNodes();
	for (int i = 0; i < pdgNodes.size(); i++) {
		isl_id *name = getNameISL(pdgNodes[i]);

		Tarjan::Node *tNode = new Tarjan::Node;
		tNode->id = name;
		tarjanNodes.push_back(tNode);

		nodeNameMap[pdgNodes[i]] = name;
	}

	// 2. Create edges
	std::vector<Tarjan::Edge*> tarjanEdges;
	pDeps_t pdgEdges = getDependences();
	for (int i = 0; i < pdgEdges.size(); i++) {
		Tarjan::Edge *tEdge = new Tarjan::Edge;
		// get tarjan node by isl_id
		for (int j = 0; j < tarjanNodes.size(); ++j) {
			if (tarjanNodes[j]->id == nodeNameMap[pdgEdges[i]->from]) {
				tEdge->from	= tarjanNodes[j];
			}
			if (tarjanNodes[j]->id == nodeNameMap[pdgEdges[i]->to]) {
				tEdge->to 		= tarjanNodes[j];
			}
		}
		if (tEdge->from == NULL || tEdge->to == NULL) {
			fprintf(stderr, "ERROR: Initializing Tarjan edge is not successful!!\n");
			return ret;
		}

		tarjanEdges.push_back(tEdge);
	}

	// 3. initialize graph
	Tarjan::Graph *tarjanGraph = new Tarjan::Graph;
	tarjanGraph->nodes = tarjanNodes;
	tarjanGraph->edges = tarjanEdges;

	//std::cout << "run tarjan" <<std::endl;
	Tarjan::SCCs_t foundSCCs;
	Tarjan::TarjanClass tc(tarjanGraph);
	tc.runTarjansAlgorithm(tarjanNodes[0], foundSCCs);
	//std::cout << "finish tarjan" <<std::endl;

	/* convert found SCCs in tarjan data structure back to ADG data structure */
	for (int i = 0; i < foundSCCs.size(); ++i) {
		PDGgraphSCC pdgSCC;
		std::vector<Tarjan::Node*> tarjanSCC = foundSCCs[i];
		for (int j = 0; j < tarjanSCC.size(); ++j) {
			assert(_nodesMap.count(tarjanSCC[j]->id) > 0);
			pNode_t *pdgNode = _nodesMap[tarjanSCC[j]->id];
			pdgSCC.push_back(pdgNode);
		}
		ret.push_back(pdgSCC);
	}

	// clean up the allocated isl_id for each node, since in
	for (std::map<pNode_t*, isl_id*>::iterator mit = nodeNameMap.begin();
			mit != nodeNameMap.end();
			++mit)
	{
		isl_id_free(mit->second);
	}

	// delete the allocated tarjan nodes and edges
	delete tarjanGraph;

	return ret;
}

bool
PDG_helper::isInSCC(const pdg_helper::pDep_t *dep){
	pdg_helper::pNode_t *srcNode = getSourceNode(dep);
	pdg_helper::pNode_t *snkNode =getSnkNode(dep);

	for (int i = 0; i < this->_SCCs.size(); ++i) {
		pdg_helper::PDGgraphSCC SCC = this->_SCCs[i];
		bool isSouce = false;
		bool isSnk	   = false;
		for (int j = 0; j < SCC.size(); ++j) {
			if (srcNode->name->s == SCC[j]->name->s) {
				isSouce = true;
			}
			if (snkNode->name->s == SCC[j]->name->s) {
				isSnk = true;
			}
		} // end SCC
		if (isSouce && isSnk) {
			return true;
		}
	} // end SCCs

	return false;
}

// Lexicographic comparison of two points
int isl_point_compare(__isl_keep isl_point *point1, __isl_keep isl_point *point2) {
	isl_int v1, v2;
	isl_int_init(v1);
	isl_int_init(v2);
	int cmp = 0;
	int pos = 0;
	while (point1 && point2) {
		if (isl_point_get_coordinate(point1, isl_dim_set, pos, &v1) &&
		    isl_point_get_coordinate(point2, isl_dim_set, pos, &v2)) {
			cmp = isl_int_cmp(v1, v2);
			if (cmp != 0) {
				break;
			}
		}
		else {
			break;
		}
		pos++;
	}
	isl_int_clear(v1);
	isl_int_clear(v2);
	return cmp;
}

// Returns true if node1 fires before node2
// TODO: current implementation is a quick hack: it should work in some common cases, but it is not correct in all cases (especially when
// "complex" array access patterns are used, e.g. a[i-j] instead of just a[i]).
bool PDG_helper::firesBefore(pdg::node const *node1, pdg::node const *node2) {
	// First try comparing the prefixes
	int prefixLength = min(node1->prefix.size(), node2->prefix.size());
	for (int i = 0; i < prefixLength; i++) {
		int cmp = node1->prefix[i] - node2->prefix[i];
		if (cmp < 0) {
			fprintf(stderr, "Assuming %s executes before %s, please verify this!\n", getFunctionName(node1).c_str(), getFunctionName(node2).c_str());
			return true;
		}
		else if (cmp > 0) {
			fprintf(stderr, "Assuming %s executes after %s, please verify this!\n", getFunctionName(node1).c_str(), getFunctionName(node2).c_str());
			return false;
		}
	}

	// If prefixes are equal, compare the lexicographic minima from both domains
	if (node1->prefix.size() == node2->prefix.size()) {
		isl_set *s1 = node1->source->get_isl_set();
		isl_set *s2 = node2->source->get_isl_set();
		s1 = isl_set_lexmin(s1);
		s2 = isl_set_lexmin(s2);
		isl_point *p1 = isl_set_sample_point(s1);
		isl_point *p2 = isl_set_sample_point(s2);
		if (isl_point_compare(p1, p2) < 0) {
			fprintf(stderr, "Assuming %s executes before %s, please verify this!\n", getFunctionName(node1).c_str(), getFunctionName(node2).c_str());
			isl_point_free(p1);
			isl_point_free(p2);
			return true;
		}
		isl_point_free(p1);
		isl_point_free(p2);
		return false;
	}
	else {
		// This case should never occur.
		fprintf(stderr, "Unhandled case for %s and %s\n", getFunctionName(node1).c_str(), getFunctionName(node2).c_str());
		return false;
	}
	return false;
}

} // namespace pdg_helper

