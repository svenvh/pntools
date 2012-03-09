/*
 * CommuFreeParition.cc
 *
 *  Created on: Jan 25, 2012
 *      Author: teddyzhai
 */

#include "global.h"
#include "assert.h"
#include <vector>

#include "isl/ctx.h"
#include "isl/set.h"
#include "isl/space.h"

#include "isa/pdg.h"
#include "isa/adg.h"
#include "adg_parse.h"

#include "ADG_helper.h"
#include "PDG_helper.h"
#include "CommuFreeParition.h"

using namespace pdg;
using namespace adg_helper;
using pdg_helper::PDG_helper;

#define COMMU_FREE_DEBUG

struct transDeps_t {
	// store all dependences in the original PDG
	pdg_helper::pDeps_t originalDeps;
	// store all transitive dependences except all original dependences
	pdg_helper::pRelations_t transRels;
};


CommuFreeParition::CommuFreeParition(ADG_helper *adg_helper, isl_ctx *ctx)
{
	this->_ctx = ctx;
//	this->excl_process_ids = excl_process_ids;
	this->_adg_helper = adg_helper;


	doTrafo();
}

CommuFreeParition::CommuFreeParition(PDG_helper *pdgHelper, isl_ctx *ctx)
{
	this->_ctx = ctx;
	this->_pdg_helper = pdgHelper;

	/* get all SCCs of the pdg */
	this->_SCCs = _pdg_helper->getSCCs();
#ifdef COMMU_FREE_DEBUG
	for (int i = 0; i < _SCCs.size(); ++i) {
		pdg_helper::PDGgraphSCC SCC = _SCCs[i];
		std::cout << "SCC: " << i << ": ";
		for (int j = 0; j < SCC.size(); ++j) {
			std::cout << SCC[j]->name->s << " ";
		}
		 std::cout << std::endl;
	}
#endif


	if (isTopoValid()) {
		doTrafoPDG();
	}
}

CommuFreeParition::CommuFreeParition(adg *adg, isl_ctx *ctx,
		std::vector<unsigned int> &excl_process_ids)
{
	this->_ctx = ctx;
	this->_adg_graph = adg;
	this->_excl_process_ids = excl_process_ids;
	this->_adg_helper = new ADG_helper(adg, ctx);

	// get all isl names from integer ids
	getExcludeNodes();

	doTrafo();
}

CommuFreeParition::~CommuFreeParition() {
//	for (int i = 0; i < _originalDeps.size(); ++i) {
//		isl_map_free(_originalDeps[i]);
//	}
}

void
CommuFreeParition::getExcludeNodes(){
	assert(_excl_process_ids.size() > 0);

	adg_helper::Nodes srcNodes = _adg_helper->getSourceNodes();
	adg_helper::Nodes snkNodes = _adg_helper->getSinkNodes();

	// for all nodes
	for (int j = 0; j < _adg_graph->nodes.size(); ++j) {
		isl_id *name = _adg_graph->nodes[j]->name;

		const char *name_char = isl_id_get_name(name);
		for (int i = 0; i < _excl_process_ids.size(); ++i) {
			unsigned int id = _excl_process_ids[i];

			int name_int = -1;
			if (sscanf(name_char, "ND_%d", &name_int) != 1) {
				fprintf(stderr, "ERROR: the integer id in node name can not be retrieved.");
			}
			assert(name_int >= 0);

			if (name_int != id) {
				continue;
			}

			_excl_processes[id] = (_adg_graph->nodes[j]);
		}
	}
}

void
CommuFreeParition::excludeSrcSnkNodes(){

}

bool
CommuFreeParition::isTopoValid(){
	assert(_pdg_helper != NULL);
	bool isValid = true;

	return isValid;
}

bool
CommuFreeParition::isInSCC(const pdg_helper::pDep_t *dep){
	pdg_helper::pNode_t *srcNode = _pdg_helper->getSourceNode(dep);
	pdg_helper::pNode_t *snkNode = _pdg_helper->getSnkNode(dep);

	for (int i = 0; i < _SCCs.size(); ++i) {
		pdg_helper::PDGgraphSCC SCC = _SCCs[i];
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

// call-back function
int doDepBMap(__isl_take isl_basic_map *dep, void *user){
	transDeps_t *allDeps = (transDeps_t*) user;
	pdg_helper::pDeps_t originalDeps = (allDeps->originalDeps);

//	std::cout << "----> trans dep (bmap): " << std::endl;
//	isl_basic_map_dump(dep);

	bool found = false;
	isl_map *dep_map = isl_map_from_basic_map(isl_basic_map_copy(dep));
	for (int i = 0; i < originalDeps.size(); ++i) {
		// the map of each original dependence is essentially basic map,
		// therefore, we choose to operate on map
		if (isl_map_is_equal(dep_map, (originalDeps)[i]->relation->map)) {
			found = true;
			break;
		}
	}

	if (found == false) {
		allDeps->transRels.push_back(dep_map);
	} else {
		isl_map_free(dep_map);
	}

	isl_basic_map_free(dep);
	return 0;
}


int doDep(__isl_take isl_map *dep, void *user){
	transDeps_t *allDeps = (transDeps_t*) user;


	int rt = isl_map_foreach_basic_map(dep, &doDepBMap, allDeps);
	assert(rt == 0);

	isl_map_free(dep);
	return 0;
}


void
CommuFreeParition::doTrafo(){
//	Nodes srcNodes = _adg_helper->getSourceNodes();
//	Nodes snkNodes = _adg_helper->getSinkNodes();

	/* perform communication-free partitioning:
	 * 1. embed the domain of each node with id as the leading entry
	 *  */
	Nodes nodes =_adg_helper->getNodes();
	isl_union_map *unionDeps = NULL;
//	for (int i = 0; i < nodes.size(); ++i) {
//		Node *node = nodes[i];
//		std::cout << "node: " << isl_id_get_name(node->name) << std::endl;
//
//		// skip source and sink nodes
//		NodeCIter nit = find(srcNodes.begin(), srcNodes.end(), node);
//		if (nit != srcNodes.end()) {
//			continue;
//		}
//		nit = find(snkNodes.begin(), snkNodes.end(), node);
//		if (nit != snkNodes.end()) {
//			continue;
//		}

//		const char *name_char = isl_id_get_name(node->name);
//		int name_int = -1;
//		if (sscanf(name_char, "ND_%d", &name_int) != 1) {
//			fprintf(stderr, "ERROR: the integer id in node name can not be retrieved.");
//		}
//		assert(name_int >= 0);

		// 1. embed the domain of each node with id as the leading entry
//		isl_set *nodeDomain = _adg_helper->getNodeDomainBound(node);
//		nodeDomain = isl_set_insert_dims(nodeDomain, isl_dim_set, 0, 1);
//		isl_local_space *lSp = isl_local_space_from_space(isl_set_get_space(nodeDomain));
//		isl_constraint *constrnt = isl_equality_alloc(lSp);
//		constrnt = isl_constraint_set_constant_si(constrnt, -name_int);
//		constrnt = isl_constraint_set_coefficient_si(constrnt, isl_dim_set, 0, 1);
//		nodeDomain = isl_set_add_constraint(nodeDomain, constrnt);

		// 2. union of all dependences
//		Edges edges = _adg_helper->getEdges();
//		// get first dependency
//		isl_map *dep0_map = isl_map_from_basic_map(
//				isl_basic_map_from_multi_aff(edges[0]->map));
//		isl_space *sp = isl_map_get_space(dep0_map);
//		unionDeps = isl_union_map_empty(sp);
//		unionDeps = isl_union_map_add_map(unionDeps, (dep0_map));
//		for (int j = 1; j < edges.size(); ++j) {
//			isl_map *dep_map= isl_map_from_basic_map(
//					isl_basic_map_from_multi_aff(edges[i]->map));
//			std::cout << "edge: " << isl_id_get_name(edges[i]->name) << " ";
//			isl_map_dump(dep_map);
//			unionDeps = isl_union_map_add_map(unionDeps, dep_map);
//			isl_union_map_dump(unionDeps);
//		}
//	} // end nodes

	// union of all dependences
//	Edges edges = _adg_helper->getEdges();
//	assert(edges.size() > 0);
//	// get first dependency to initialize the union of dependences
//	isl_map *dep0_map = isl_map_from_basic_map(
//			isl_basic_map_from_multi_aff(edges[0]->map));
//	_originalDeps.push_back(isl_map_copy(dep0_map));
//	std::cout << isl_id_get_name(edges[0]->name) << ": " << std::endl;
//	isl_map_dump(dep0_map);
//	isl_space *sp = isl_map_get_space(dep0_map);
//	unionDeps = isl_union_map_empty(sp);
//	unionDeps = isl_union_map_add_map(unionDeps, (dep0_map));
//	for (int j = 1; j < edges.size(); ++j) {
//		isl_map *dep_map= isl_map_from_basic_map(
//				isl_basic_map_from_multi_aff(edges[j]->map));
//		std::cout << isl_id_get_name(edges[j]->name) << ": " << std::endl;
//		isl_map_dump(dep_map);
//		_originalDeps.push_back(isl_map_copy(dep_map));
//		unionDeps = isl_union_map_add_map(unionDeps, dep_map);
//	}
//	assert(unionDeps != NULL);
//	std::cout << "--->union of all deps: " << std::endl ;
//	isl_union_map_dump(unionDeps);
//
//	// 3. perform transitive closure on the obtained union
//	int exact = -1;
//	unionDeps = isl_union_map_transitive_closure(unionDeps, &exact);
//	std::cout << "---> transitive depencecies :" << std::endl;
//	isl_union_map_dump(unionDeps);
//
////	doDep(_originalDeps[0], &_originalDeps);
//
//
////	int rt = isl_union_map_foreach_map(unionDeps, &doDep, &originalDeps);
////	assert(rt == 0);
//
//	isl_union_map_free(unionDeps);

}

/* perform transform on PDG data structure.
 * It has the advantage that all maps are not nested and are easier than those in ADG to manipulate*/
void
CommuFreeParition::doTrafoPDG(){
	pdg_helper::pNodes_t nodes = _pdg_helper->getNodes();
	pdg_helper::pDeps_t originalDeps = _pdg_helper->getDependences();
	unsigned int nrInvEdge = 0; // number of added dependency edge

	/* build a union of all dependences */
	assert(originalDeps.size() > 0);
	isl_map *dep0_map = originalDeps[0]->relation->map;
	isl_union_map *unionDep = isl_union_map_from_map(isl_map_copy(dep0_map));
#ifdef COMMU_FREE_DEBUG
	std::cout << "dep 0:" << std::endl;
	isl_map_dump(originalDeps[0]->relation->map);
#endif

	// add an inverse relation if the dependence edge is not involved in any SCC
	if (!isInSCC(originalDeps[0])) {
		isl_map *inv_dep = isl_map_reverse(isl_map_copy(dep0_map));
		unionDep = isl_union_map_add_map(unionDep, inv_dep);
		nrInvEdge++;
	}

	// for the rest of dependency edges
	for (int i = 1; i < originalDeps.size(); ++i) {
		isl_map *dep_map = originalDeps[i]->relation->map;
#ifdef COMMU_FREE_DEBUG
		std::cout << "->dep: " << i << std::endl;
		isl_map_dump(originalDeps[i]->relation->map);
#endif
		unionDep = isl_union_map_add_map(unionDep, isl_map_copy(dep_map));

		// if the edge is not involved in any SCC, its inverse is also added
		if (!isInSCC(originalDeps[i])) {
			isl_map *inv_dep = isl_map_reverse(isl_map_copy(dep_map));
			unionDep = isl_union_map_add_map(unionDep, inv_dep);
			nrInvEdge++;
		}
	}
#ifdef COMMU_FREE_DEBUG
	std::cout << "nr. nodes: " << nodes.size() << std::endl;
	std::cout << "nr. added edges: " << nrInvEdge << std::endl;
	/*std::cout << "->union of all deps: " << std::endl ;
	isl_union_map_dump(unionDep);*/
#endif

	/* compute transitive dependences of the PDG */
	int exact = -1;
	unionDep = isl_union_map_transitive_closure(unionDep, &exact);
//	std::cout << "-> transitive depencecies :" << std::endl;
//	isl_union_map_dump(unionDep);
	if (exact == 1) {
		std::cout << "-> performing exact transitive closure computation." << std::endl;
	} else {
		std::cout << "-> performing APPROXIMATION transitive closure computation." << std::endl;
	}

	/* find all transitive dependences mapped to source nodes
	 * to determine how many communication-free partitions can be derived.*/
//	pdg_helper::pNodes_t pSrcNodes;
//	pdg_helper::pNodes_t pSnkNodes;
//	_pdg_helper->getSrcSnkNodes(&pSrcNodes, &pSnkNodes);
//	assert(pSrcNodes.size() > 0 && pSnkNodes.size() > 0);
//	std::vector<const char*> srcNodesNames;
//	std::cout << "-> source nodes are: ";
//	for (int i = 0; i < pSrcNodes.size(); ++i) {
//		const char *name_char =pSrcNodes[i]->name->s.c_str();
//		std::cout << " " << name_char;
//		srcNodesNames.push_back(name_char);
//	}
//	std::cout << std::endl;
//	std::vector<const char*> snkNodesNames;
//	std::cout << "-> sink nodes are: ";
//	for (int i = 0; i < pSnkNodes.size(); ++i) {
//		const char *name_char =pSnkNodes[i]->name->s.c_str();
//		std::cout << " " << name_char;
//		snkNodesNames.push_back(name_char);
//	}
//	std::cout<< std::endl;
//
//	/* check, for each (transitive) dependence, */
//	transDeps_t *allDeps = new transDeps_t;
//	allDeps->originalDeps = originalDeps;
//	int rt = isl_union_map_foreach_map(unionDep, &doDep, allDeps);
//	assert(rt == 0);
//	std::cout << "-> all transitive dependences: " << std::endl;
//	for (int i = 0; i < allDeps->transRels.size(); ++i) {
//		isl_map_dump(allDeps->transRels[i]);
//	}

//	// for each non-source and non-sink node, find the transitive dependence to the source nodes
//	pdg_helper::pNodes_t dataflowNodes = _pdg_helper->getDataflowNodes();
//	std::vector<const char*> dataflowNodesNames;
//	for (int i = 0; i < dataflowNodes.size(); ++i) {
//		const char *name = dataflowNodes[i]->name->s.c_str();
//		dataflowNodesNames.push_back(name);
//	}
//	for (int i = 0; i < allDeps->transRels.size(); ++i) {
//		bool isSrc = false;
//		bool isSnk = false;
//
//		// get the id of the domain in the map
////		const char *name_char_in = isl_map_get_tuple_name(allDeps->transRels[i], isl_dim_in);
////		std::vector<const char*>::iterator cit =
////				find(srcNodesNames.begin(), srcNodesNames.end(), name_char_in);
////		if (cit != srcNodesNames.end()) {
////			isSrc = true;
////		}
//		// get the id of the range in the map
////		const char *name_char_out = isl_map_get_tuple_name(allDeps->transRels[i], isl_dim_out);
////		cit =	find(snkNodesNames.begin(), snkNodesNames.end(), name_char_out);
////		if (cit != snkNodesNames.end()) {
////			isSnk = true;
////		}
//	}

	// free all transitive relations
//	for (int i = 0; i < allDeps->transRels.size(); ++i) {
//		isl_map_free(allDeps->transRels[i]);
//	}
//	delete allDeps;
	isl_union_map_free(unionDep);
}

