/*
 * Copyright (c) 2012 Leiden University (LERC group at LIACS).
 * All rights reserved.
 *
 * ADG_helper.h
 *
 *  Created on: Jan 18, 2012
 *      Author: Teddy Zhai
 */

#ifndef ADG_HELPER_H_
#define ADG_HELPER_H_

#include "global.h"
#include "defs.h"

#include "isa/adg.h"

namespace adg_helper {

class ADG_helper {
	isl_ctx *ctx;
	adg* ppn;
	CDNodeIds cdNodeIds;

	NameId_t adg_ids; // used for csdf data structure

public:
	ADG_helper(adg *adg, isl_ctx *ctx);
	// a adg and a set of adg nodes to be merged into a single CDNode
	ADG_helper(adg *adg, CDNodeIds &cdNodeIds, isl_ctx *ctx);
	virtual ~ADG_helper();

	Processes getProcesses();
	Node* getNode(isl_id *name);
	Node* getNode(id_t id);

	Channels getChannels();
	Channel* getChannel(isl_id *name);

	Edge* getEdge(isl_id *name);

	unsigned getId(const Process *process);
	unsigned getId(const Port *port);
	unsigned getId(isl_id *name);
	unsigned getNewId();

	__isl_give isl_set* getProcessDomainBound(const Node*);
	__isl_give isl_set* getCDNodeDomainBound(const CDNode*);

	Ports getInPorts(const Process *process);
	Ports getInPorts(const CDNode *cdNode);

	Ports getOutPorts(const Process *process);
	Ports getOutPorts(const CDNode *cdNode);

	__isl_give isl_set* getPortDomainBound(const Port *port);

	std::vector<adg_param*> getParameters();

	bool isSelfEdge(Edge *ch, const CDNode *cdNode);

	// topological analysis
	bool isChain();
	bool isChain(const Processes &processes);
	bool isTree();
	bool isTree(const Processes &processes);
};

} // end namespace adg_helper

#endif /* ADG_HELPER_H_ */
