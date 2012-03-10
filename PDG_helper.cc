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

} // namespace pdg_helper

