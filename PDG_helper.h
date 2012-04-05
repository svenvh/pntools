/*		Copyright (c) 2012 Leiden University (LERC group at LIACS).
 * 		All rights reserved.
 *
 *		PDG_helper.h
 *
 *  	Created on: Feb 7, 2012
 *      Author: Teddy Zhai
 */

#ifndef PDG_HELPER_H_
#define PDG_HELPER_H_

#include "defs.h"

//using namespace pdg;
using pdg::PDG;

//namespace pdg{

namespace pdg_helper {

class PDG_helper {
	isl_ctx *_ctx;
	pdg::PDG *_pdg;

	std::map<isl_id*, pNode_t*> _nodesMap;

	pNodes_t _srcNodes;
	pNodes_t _snkNodes;
public:
	PDG_helper(isl_ctx *ctx, PDG *pdg);
	virtual ~PDG_helper();

	pNodes_t getNodes();
	pNode_t* getNode(const std::string&);
	pNode_t* getSourceNode(const pDep_t*);
	pNode_t* getSnkNode(const pDep_t*);

	bool isSourceNode(const pNode_t*);
	bool isSinkNode(const pNode_t*);

	void getSrcSnkNodes(pNodes_t*, pNodes_t*);
	pNodes_t getDataflowNodes();

	__isl_give isl_id* getNameISL(const pNode_t*);
	std::string getFunctionName(pdg::node const *node);

	pDeps_t getDependences();
	bool isSelfDependence(const pDep_t*);

	// topology
	bool isChain();
	bool isTree();

	PDGgraphSCCs getSCCs();

	bool isInSCC(const pdg_helper::pDep_t *dep);

};

}
//}
#endif /* PDG_HELPER_H_ */
