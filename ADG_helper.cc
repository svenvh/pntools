/*
 * Copyright (c) 2012 Leiden University (LERC group at LIACS).
 * All rights reserved.
 *
 * ADG_helper.cc
 *
 *  Created on: Jan 18, 2012
 *      Author: Teddy Zhai
 */

#include "defs.h"

#include "isl/set.h"

#include "tarjan.h"
#include "ADG_helper.h"

using namespace adg_helper;

namespace adg_helper {

ADG_helper::ADG_helper(adg *adg, isl_ctx *ctx) {
	this->ppn = adg;
	this->ctx = ctx;

	// initialize a unique id for each process as well for port
	Processes processes = ppn->nodes;
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

ADG_helper::ADG_helper(adg *adg, CDNodeIds &cdNodeIds, isl_ctx *ctx) {
	this->ppn = adg;
	this->cdNodeIds = cdNodeIds;
	this->ctx = ctx;

	// initialize a unique id for each process as well for port
	Processes processes = ppn->nodes;
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

	/* initialize CDNodes */
	// initialize new names for CDNodes
//	for (int i = 0; i < cdNodeIds.size(); ++i) {
//		CDNode cdNode = cdNodeIds[i];
//		isl_id *nodeName = NULL;
//		char* cdNodeNameChar;
//		for (int j = 0; j < cdNode.size(); ++j) {
//			// get each id of Node in the CDNode
//			const char* nameChar = isl_id_get_name(cdNode[j]->name);
//			strcat(cdNodeNameChar, nameChar);
//		}
//	}




}

ADG_helper::~ADG_helper() {
	// destructor for CDNodes

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

unsigned
ADG_helper::getNewId(){
	assert(adg_ids.size() > 0);
	return adg_ids.size() + 1;
}

unsigned
ADG_helper::getIdfromName(isl_id *name){
//	int name_int = -1;
//	if (sscanf(name_char, "ND_%d", &name_int) != 1) {
//		fprintf(stderr, "ERROR: the integer id in node name can not be retrieved.");
//	}
//	assert(name_int >= 0);
}

Channels
ADG_helper::getChannels(){
		return this->ppn->edges;
}

Channel*
ADG_helper::getChannel(isl_id *name){
	const char* name_char = isl_id_get_name(name);

	Channels channels = ppn->edges;
	for (int i = 0; i < channels.size(); ++i) {
		const char *ch_name_char = isl_id_get_name(channels[i]->name);

		if (strcmp(name_char, ch_name_char)) continue;

		return channels[i];
	}

	// should not reach here
	fprintf(stderr, "channel with name: %s is not found!!", name_char);
}

Edges
ADG_helper::getEdges(){
	return this->ppn->edges;
}

Edge*
ADG_helper::getEdge(isl_id *name){
	Edges edges = ppn->edges;
	for (int i = 0; i < edges.size(); ++i) {
		isl_id *name_tmp = edges[i]->name;

		if (name != name_tmp) continue;

		return edges[i];
	}

	// should not reach here
	fprintf(stderr, "edge with name: %s is not found!!", isl_id_get_name(name));
}

Edges
ADG_helper::getSelfEdges(const Node *node){
	Edges edges;

	Ports in_ports = node->input_ports;
	for (int i = 0; i < in_ports.size(); ++i) {
		Edge *edge = getEdge(in_ports[i]->edge_name);
		if ( isSelfEdge(edge) ) {
			edges.push_back(edge);
		}
	}

	return edges;
}

Edges
ADG_helper::getNodeEdges(const Node *node){
	Edges edges;

//	Ports in_ports = node->input_ports;
//	for (int i = 0; i < in_ports.size(); ++i) {
//		Edge *edge = getEdge(in_ports[i]->edge_name);
//		if ( isSelfEdge(edge) ) {
//			edges.push_back(edge);
//		}
//	}

	return edges;
}

Processes
ADG_helper::getProcesses(){
	return this->ppn->nodes;
}

Nodes
ADG_helper::getNodes(){
	return this->ppn->nodes;
}

Node*
ADG_helper::getNode(isl_id *name){
	Nodes nodes = ppn->nodes;
	for (int i = 0; i < nodes.size(); ++i) {
		isl_id *name_tmp = nodes[i]->name;

		if (name != name_tmp) continue;

		return nodes[i];
	}

	// should not reach here
	fprintf(stderr, "node with name: %s is not found!!", isl_id_get_name(name));
}

Node*
ADG_helper::getNode(id_t id){
	// get node name in isl_id
	for (NameId_t::const_iterator nit = adg_ids.begin(); nit != adg_ids.end(); ++nit) {
		if (nit->second != id) continue;

		isl_id *name = nit->first;
		for (int i = 0; i < ppn->nodes.size(); ++i) {
			if (ppn->nodes[i]->name != name) continue;

			return ppn->nodes[i];
		}
	}

	// should not reach here
	fprintf(stderr, "node with id: %d is not found!!", id);

}

Nodes
ADG_helper::getSourceNodes(){
	Nodes srcNodes;

	for (int i = 0; i < ppn->nodes.size(); ++i) {
		Ports ports = getInPorts(ppn->nodes[i]);

		if (ports.size() == 0) {
			srcNodes.push_back(ppn->nodes[i]);
		}
	}

	return srcNodes;
}

Nodes
ADG_helper::getSinkNodes(){
	Nodes snkNodes;

	for (int i = 0; i < ppn->nodes.size(); ++i) {
		Ports ports = getOutPorts(ppn->nodes[i]);

		if (ports.size() == 0) {
			snkNodes.push_back(ppn->nodes[i]);
		}
	}

	return snkNodes;
}

__isl_give isl_set*
ADG_helper::getProcessDomainBound(const Process *process){
	isl_set *process_domain = isl_set_copy(process->domain->bounds);

	return process_domain;
}

__isl_give isl_set*
ADG_helper::getNodeDomainBound(const Node *node){
	isl_set *nodeDom= isl_set_copy(node->domain->bounds);

	return nodeDom;
}

Port*
ADG_helper::getPort(isl_id *name){

}

Port*
ADG_helper::getSrcPort(Edge *edge){
	isl_id *port_name = edge->from_port_name;
	Node *node = getNode(edge->from_node_name);
	Ports out_ports = node->output_ports;
//	PortIter::pit = find(in_ports.begin(), in_ports.end(), edge->from_port_name);
	for (int i = 0; i < out_ports.size(); ++i) {
		if (out_ports[i]->name != port_name) continue;

		return out_ports[i];
	}

	fprintf(stderr, "ERROR: source port: %s of edge %s dose not exit.",
			isl_id_get_name(edge->from_port_name), isl_id_get_name(edge->name));
}


Port*
ADG_helper::getSnkPort(Edge *edge){
	isl_id *port_name = edge->to_port_name;
	Node *node = getNode(edge->to_node_name);
	Ports in_ports = node->input_ports;
//	PortIter::pit = find(in_ports.begin(), in_ports.end(), edge->from_port_name);
	for (int i = 0; i < in_ports.size(); ++i) {
		if (in_ports[i]->name != port_name) continue;

		return in_ports[i];
	}

	fprintf(stderr, "ERROR: source port: %s of edge %s dose not exit.",
			isl_id_get_name(edge->from_port_name), isl_id_get_name(edge->name));
}

Ports
ADG_helper::getInPorts(const Process *process){
	return process->input_ports;
}


Ports
ADG_helper::getOutPorts(const Process *process){
	return process->output_ports;
}

Ports
ADG_helper::getConnectedPort(Port* port){
	// get
}

__isl_give isl_set*
ADG_helper::getPortDomainBound(const Port *port){
	isl_set *port_domain = isl_set_copy(port->domain->bounds);

	return port_domain;
}

std::vector<adg_param*>
ADG_helper::getParameters(){
	return this->ppn->params;
}

// a simple check if the adg has a chain topology
bool
ADG_helper::isChain(){
	bool isChain = true;

	Nodes processes = this->ppn->nodes;
	Nodes srcNodes = getSourceNodes();
	Nodes snkNodes = getSinkNodes();
	// if each process has at most one input/output port, then adg is a chain
	for (int i = 0; i < processes.size(); ++i) {
		Node *process = processes[i];

		// skip source and sink nodes
		NodeIter nit = find(srcNodes.begin(), srcNodes.end(), process);
		if (nit != srcNodes.end()) {
			continue;
		}
		nit = find(snkNodes.begin(), snkNodes.end(), process);
		if (nit != snkNodes.end()) {
			continue;
		}

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

		// if several output ports exist, it is not a chain for sure
		Ports out_ports = process->output_ports;
		if (out_ports.size() > 1) {
			return false;
		}
	}

	return isChain;
}


// a simple check if the adg has a chain topology
bool
ADG_helper::isTree(){
	bool isTree = true;

	Processes processes = this->ppn->nodes;

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

// bound of a CDNode is union of all bounds of all merged adg nodes
__isl_give isl_set*
ADG_helper::getCDNodeDomainBound(const CDNode *cdNode){
	assert(cdNode->size() != 0);

	isl_set *boundCdNode = isl_map_range( isl_map_copy((*cdNode)[0]->schedule) );

	for (int i = 1; i < (*cdNode).size(); ++i) {
		isl_set *boundAdgNode = isl_map_range( isl_map_copy( (*cdNode)[i]->schedule ) );

		boundCdNode = isl_set_union(boundCdNode, boundAdgNode);
	}

	return boundCdNode;
}

bool
ADG_helper::isSelfEdge(Edge *ch){
	bool isSelfEdge = true;

	if (ch->from_node_name != ch->to_node_name) {
		isSelfEdge = false;
	}

	return isSelfEdge;
}

bool
ADG_helper::isSelfEdge(Edge *ch, const CDNode *cdNode){
	bool isSelfEdge = true;

	Node *from_node = getNode(ch->from_node_name);
	if (find((*cdNode).begin(), (*cdNode).end(), from_node) == (*cdNode).end()) {
		isSelfEdge = false;
		return isSelfEdge;
	}

	Node *to_node = getNode(ch->to_node_name);
	if (find(cdNode->begin(), cdNode->end(), to_node) == cdNode->end()) {
		isSelfEdge = false;
		return isSelfEdge;
	}

	return isSelfEdge;
}

Ports
ADG_helper::getInPorts(const CDNode *cdNode){
	Ports ports;

	// for all adg nodes
	for (int i = 0; i < cdNode->size(); ++i) {
		Ports in_ports = (*cdNode)[i]->input_ports;
		// for all input ports
		for (int j = 0; j < in_ports.size(); ++j) {
			Port *port = in_ports[j];

			// check if the connected edge is a self-edge
			Edge *edge = getEdge(port->edge_name);

			if (isSelfEdge(edge, cdNode)) continue;

			ports.push_back(port);
		} // input
	}

	return ports;
}


Ports
ADG_helper::getOutPorts(const CDNode *cdNode){
	Ports ports;

	// for all adg nodes
	for (int i = 0; i < cdNode->size(); ++i) {
		Ports out_ports = (*cdNode)[i]->output_ports;
		// for all output ports
		for (int j = 0; j < out_ports.size(); ++j) {
			Port *port = out_ports[j];

			// check if the connected edge is a self-edge
			Edge *edge = getEdge(port->edge_name);

			if (isSelfEdge(edge, cdNode)) continue;

			ports.push_back(port);
		} // input
	}

	return ports;
}


ADGgraphSCCs
ADG_helper::getSCCs(){
	ADGgraphSCCs ret;
	Tarjan::TarjanClass tc;
	ret = tc.runTarjansAlgorithm(this);

	 return ret;
}

} // end namespace adg_helper
