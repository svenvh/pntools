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

struct ind_dims_t {
	unsigned int dims;
	int nr_ind_dim;
	std::map<unsigned int, bool> dim_dep_map;
};

typedef std::map<std::string, std::vector<short>* > phaseMap_t;

typedef std::map<isl_id*, isl_set*> var_domain_t;

typedef std::map<isl_id*, std::vector<short>* > phases_t;

class ADG_helper {
	isl_ctx *ctx;
	adg* ppn;
	phases_t phases;
	var_domain_t var_domains;
	CDNodeIds cdNodeIds;

	NameId_t adg_ids; // used for csdf data structure

public:
	ADG_helper(adg *adg, isl_ctx *ctx);
	// a adg and a set of adg nodes to be merged into a single CDNode
	ADG_helper(adg *adg, CDNodeIds &cdNodeIds, isl_ctx *ctx);
	virtual ~ADG_helper();

	Processes getProcesses();
	Nodes getNodes();
	Node* getNode(isl_id *name);
	Node* getNode(id_t id);
	Nodes getSourceNodes();
	Nodes getSinkNodes();

	Channels getChannels();
	Channel* getChannel(isl_id *name);

	Edges getEdges();
	Edge* getEdge(isl_id *name);
	Edges getNodeEdges(const Node *node);
	Edges getSelfEdges(const Node *node);


	unsigned getId(const Process *process);
	unsigned getId(const Port *port);
	unsigned getId(isl_id *name);
	unsigned getIdfromName(isl_id*);
	unsigned getNewId();

	__isl_give isl_set* getProcessDomainBound(const Node*);
	__isl_give isl_set* getNodeDomainBound(const Node*);
	__isl_give isl_set* getCDNodeDomainBound(const CDNode*);

	Port* getSrcPort(Edge *edge);
	Port* getSnkPort(Edge *edge);

	Port* getPort(isl_id *name);

	Ports getInPorts(const Process *process);
	Ports getInPorts(const CDNode *cdNode);

	Ports getOutPorts(const Process *process);
	Ports getOutPorts(const CDNode *cdNode);


	Ports getConnectedPort(Port*);

	__isl_give isl_set* getPortDomainBound(const Port *port);

	std::vector<adg_param*> getParameters();

	bool isSelfEdge(Edge *ch, const CDNode *cdNode);
	bool isSelfEdge(Edge *ch);


	// topological analysis
	bool isChain();
	bool isChain(const Processes &processes);
	bool isTree();
	bool isTree(const Processes &processes);

	ADGgraphSCCs getSCCs();

	// Phase/pattern computation
	void initPhases();
	void writePhase(isl_id *portName, std::ostream &strm, char sep);
	int getCommonPortsDims(__isl_keep isl_set *process_domain, Ports &ports, ind_dims_t &ind_dims);
	var_domain_t *findVariantDomain2(const Process *process);
	unsigned int getPhaseLength(__isl_keep isl_set *var_domain);
	phases_t *computePhases(const Process *process);
	bool checkSimplePattern(const Process *process);
};

} // end namespace adg_helper

#endif /* ADG_HELPER_H_ */
