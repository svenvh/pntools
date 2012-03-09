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

public:
	PDG_helper(isl_ctx *ctx, PDG *pdg);
	virtual ~PDG_helper();

	pNodes_t getNodes();
	pNode_t* getNode(const std::string&);
	pNode_t* getSourceNode(const pDep_t*);
	pNode_t* getSnkNode(const pDep_t*);

	void getSrcSnkNodes(pNodes_t*, pNodes_t*);
	pNodes_t getDataflowNodes();

	__isl_give isl_id* getNameISL(const pNode_t*);

	pDeps_t getDependences();

	// topology
	bool isChain();
	bool isTree();

	PDGgraphSCCs getSCCs();

};

}
//}
#endif /* PDG_HELPER_H_ */
