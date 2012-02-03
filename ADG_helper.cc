/*
 * ADG_helper.cc
 *
 *  Created on: Jan 18, 2012
 *      Author: Teddy Zhai
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

unsigned
ADG_helper::getId(isl_id *name){
	assert(adg_ids.count(name) > 0);
	return adg_ids[name];
}

Channels
ADG_helper::getChannels(){
		return this->ADG->edges;
}

Channel*
ADG_helper::getChannel(isl_id *name){
	const char* name_char = isl_id_get_name(name);

	Channels channels = ADG->edges;
	for (int i = 0; i < channels.size(); ++i) {
		const char *ch_name_char = isl_id_get_name(channels[i]->name);

		if (strcmp(name_char, ch_name_char)) continue;

		return channels[i];
	}

	// should not reach here
	fprintf(stderr, "channel with name: %s is not found!!", name_char);
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

// a simple check if the adg has a chain topology
bool
ADG_helper::isChain(){
	bool isChain = true;

	Processes processes = this->ADG->nodes;

	// if each process has at most one input/output port, then adg is a chain
	for (int i = 0; i < processes.size(); ++i) {
		Process *process = processes[i];

		Ports in_ports = process->input_ports;
		if (in_ports.size() > 1) {
			// further check: if all ports are connected to the source node, it is still a chain
			Channel *ch0 = getChannel(in_ports[0]->edge_name);
			const char *from_node_name_0 = isl_id_get_name(ch0->from_node_name);

			for (int j = 1; j < in_ports.size(); ++j) {
				Channel *ch = getChannel(in_ports[i]->edge_name);
				const char *from_node_name = isl_id_get_name(ch->from_node_name);
				if (strcmp(from_node_name_0, from_node_name)) {
					return false;
				}
			}
		}

		Ports out_ports = process->output_ports;
		if (out_ports.size() > 1) {
			return false;
		}
	}

	return isChain;
}

bool
ADG_helper::isTree(const Processes &processes){

}

// a simple check if the adg has a chain topology
bool
ADG_helper::isTree(){
	bool isTree = true;

	Processes processes = this->ADG->nodes;

	// if each process has at most one input, then adg is a tree
	for (int i = 0; i < processes.size(); ++i) {
		Process *process = processes[i];

		Ports in_ports = process->input_ports;
		if (in_ports.size() > 1) {
			return false;
		}

		Ports out_ports = process->output_ports;
		if (out_ports.size() > 1) {
			// further check: if all ports are connected to the source node, it is still a chain
			Channel *ch0 = getChannel(out_ports[0]->edge_name);
			const char *to_node_name_0 = isl_id_get_name(ch0->to_node_name);
			for (int j = 1; j < out_ports.size(); ++j) {
				Channel *ch = getChannel(out_ports[i]->edge_name);
				const char *to_node_name = isl_id_get_name(ch->to_node_name);
				if (strcmp(to_node_name_0, to_node_name)) {
					return false;
				}
			}
		}
	}

	return isTree;
}
