/*
 * ADG_helper.h
 *
 *  Created on: Jan 18, 2012
 *      Author: Teddy Zhai
 *
 *      $Id: ADG_helper.h,v 1.3 2012/01/23 11:08:40 tzhai Exp $
 */

#ifndef ADG_HELPER_H_
#define ADG_HELPER_H_

#include "global.h"
#include "defs.h"

#include "isa/adg.h"

namespace adg_helper {

class ADG_helper {
	isl_ctx *ctx;
	adg* ADG;

	std::map<isl_id*, unsigned> adg_ids; // used for csdf data structure
public:
	ADG_helper(adg *ADG, isl_ctx *ctx);
	virtual ~ADG_helper();

	Processes getProcesses();
	Channels getChannels();
	Channel* getChannel(isl_id *name);


	unsigned getId(const Process *process);
	unsigned getId(const Port *port);
	unsigned getId(isl_id *name);

	__isl_give isl_set* getProcessDomainBound(const Process *process);

	Ports getInPorts(const Process *process);
	Ports getOutPorts(const Process *process);

	__isl_give isl_set* getPortDomainBound(const Port *port);

	std::vector<adg_param*> getParameters();

	// topological analysis
	bool isChain();
	bool isChain(const Processes &processes);
	bool isTree();
	bool isTree(const Processes &processes);
};

}

#endif /* ADG_HELPER_H_ */
