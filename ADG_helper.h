/*
 * ADG_helper.h
 *
 *  Created on: Jan 18, 2012
 *      Author: Teddy Zhai
 *
 *      $Id: ADG_helper.h,v 1.2 2012/01/23 10:54:37 tzhai Exp $
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

	unsigned getId(const Process *process);
	unsigned getId(const Port *port);

	__isl_give isl_set* getProcessDomainBound(const Process *process);

	Ports getInPorts(const Process *process);
	Ports getOutPorts(const Process *process);

	__isl_give isl_set* getPortDomainBound(const Port *port);

	std::vector<adg_param*> getParameters();
};

}

#endif /* ADG_HELPER_H_ */
