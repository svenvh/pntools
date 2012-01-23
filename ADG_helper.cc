/*
 * ADG_helper.cc
 *
 *  Created on: Jan 18, 2012
 *      Author: Teddy Zhai
 *
 *		$Id: ADG_helper.cc,v 1.2 2012/01/23 10:54:37 tzhai Exp $
 */

#include "ADG_helper.h"

using namespace adg_helper;

ADG_helper::ADG_helper(adg *adg, isl_ctx *ctx) {
	this->ADG = adg;

	this->ctx = ctx;

	Processes processes = ADG->nodes;
	for (int i = 0; i < processes.size(); ++i) {
		isl_id *process_id = processes[i]->name;

		this->adg_ids[process_id] = i;

		Ports in_ports = processes[i]->input_ports;
		for (int j = 0; j < in_ports.size(); ++j) {
			isl_id *port_id = in_ports[j]->name;

			this->adg_ids[port_id] = j;
		}

		Ports out_ports = processes[i]->output_ports;
		for (int j = 0; j < out_ports.size(); ++j) {
			isl_id *port_id = out_ports[j]->name;

			this->adg_ids[port_id] = in_ports.size() + 1 + j; // make it unique
		}
	}

}

ADG_helper::~ADG_helper() {
	// TODO Auto-generated destructor stub
}

unsigned
ADG_helper::getId(const Process *process){
	assert(adg_ids.count(process->name) > 0);
	return adg_ids[process->name];
}

unsigned
ADG_helper::getId(const Port *port){
	assert(adg_ids.count(port->name) > 0);
	return adg_ids[port->name];
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
