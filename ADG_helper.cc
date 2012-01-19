/*
 * ADG_helper.cc
 *
 *  Created on: Jan 18, 2012
 *      Author: Teddy Zhai
 *
 *		$Id: ADG_helper.cc,v 1.1 2012/01/19 08:59:56 tzhai Exp $
 */

#include "ADG_helper.h"

using namespace adg_helper;

ADG_helper::ADG_helper(adg *adg, isl_ctx *ctx) {
	this->ADG = adg;

	this->ctx = ctx;

}

ADG_helper::~ADG_helper() {
	// TODO Auto-generated destructor stub
}


Channels
ADG_helper::getChannels(){
		return this->ADG->edges;
}

Processes
ADG_helper::getProcesses(){
	return this->ADG->nodes;
}


__isl_give isl_set*
ADG_helper::getProcessDomainBound(const Process *process){
	isl_set *process_domain = isl_set_copy(process->domain->bounds);

	return process_domain;
}

Ports
ADG_helper::getInPorts(const Process *process){
	return process->input_ports;
}


Ports
ADG_helper::getOutPorts(const Process *process){
	return process->output_ports;
}


__isl_give isl_set*
ADG_helper::getPortDomainBound(const Port *port){
	isl_set *port_domain = isl_set_copy(port->domain->bounds);

	return port_domain;
}

std::vector<adg_param*>
ADG_helper::getParameters(){
	return this->ADG->params;
}
