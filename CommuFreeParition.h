/*
 * CommuFreeParition.h
 *
 *  Created on: Jan 25, 2012
 *      Author: Teddy Zhai
 */

#ifndef COMMUFREEPARITION_H_
#define COMMUFREEPARITION_H_

#include "isa/pdg.h"
#include "isa/adg.h"

#include "defs.h"

#include "ADG_helper.h"
#include "PDG_helper.h"

using pdg_helper::PDG_helper;

namespace adg_helper{

typedef std::map <unsigned int, adg_helper::Node* > id_node_t;
// for each node, the domain is extend for the transformation purpose
typedef std::map <unsigned int, adg_helper::Node* > node_extDomaini_t;

typedef std::vector<isl_map*> originalDeps_t;

class CommuFreeParition {
	isl_ctx *_ctx;

	adg *_adg_graph;
	adg_helper::ADG_helper *_adg_helper;
	std::vector<unsigned int> _excl_process_ids;
	std::map <unsigned int, adg_helper::Node* > _excl_processes;
//	pdg::PDG *_pdg;
	pdg_helper::PDG_helper  *_pdg_helper;
	pdg_helper::PDGgraphSCCs _SCCs;

	node_extDomaini_t node_ext_domain;
//	originalDeps_t _originalDeps;

public:
	CommuFreeParition();
	CommuFreeParition(ADG_helper *adg_helper, isl_ctx *ctx);
	CommuFreeParition(PDG_helper*, isl_ctx*);
	/* partition a adg excluding a set of nodes given in excl_process_ids  */
	CommuFreeParition(adg *adg, isl_ctx *ctx, std::vector<unsigned int> &excl_process_ids);
	virtual ~CommuFreeParition();

private:
	void getExcludeNodes();
	// exclude source and sink nodes for transformation
	void excludeSrcSnkNodes();

	bool isTopoValid();

	bool isInSCC(const pdg_helper::pDep_t *dep);

	void doTrafo();
	void doTrafoPDG();
};

} // end namespace adg_helper
#endif /* COMMUFREEPARITION_H_ */
