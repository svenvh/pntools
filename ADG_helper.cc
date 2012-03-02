/*
 * Copyright (c) 2012 Leiden University (LERC group at LIACS).
 * All rights reserved.
 *
 * ADG_helper.cc
 *
 *  Created on: Jan 18, 2012
 *      Author: Teddy Zhai
 */

#include <isl/set.h>
#include <barvinok/barvinok.h>

#include "defs.h"
#include "utility.h"

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

	initPhases();
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

	initPhases();

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

  // Free variant domains
  for (var_domain_t::iterator mit = var_domains.begin();
      mit != var_domains.end();
      ++mit)
  {
    isl_set_free(mit->second);
  }

  // deallocate allocated sets during computing phases for each port
  for (phases_t::iterator it = phases.begin(); it != phases.end(); it++) {
    delete it->second;
  }
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

// Returns all edges that have the given node as source or destination.
// This includes selfedges.
Edges
ADG_helper::getNodeEdges(const Node *node){
	Edges edges;

	int n = this->ppn->edges.size();
	for (int i = 0; i < n; i++) {
		Edge *edge = this->ppn->edges[i];
		if (isl_id_cmp(edge->from_node_name, node->name) == 0) {
			edges.push_back(edge);
		}
		else if (isl_id_cmp(edge->to_node_name, node->name) == 0) {
			edges.push_back(edge);
		}
	}

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

	/* Initialize tarjan graph using adg */
	// 1. Create nodes
	std::vector<Tarjan::Node*> tarjanNodes;
	Nodes adgNodes = getNodes();
	for (int i = 0; i < adgNodes.size(); i++) {
		Tarjan::Node *tNode = new Tarjan::Node;
		tNode->id = adgNodes[i]->name;
		tarjanNodes.push_back(tNode);
	}

	// 2. Create edges
	std::vector<Tarjan::Edge*> tarjanEdges;
	Edges adgEdges = getEdges();
	for (int i = 0; i < adgEdges.size(); i++) {
		Tarjan::Edge *tEdge = new Tarjan::Edge;
		// get tarjan node by isl_id
		for (int j = 0; j < tarjanNodes.size(); ++j) {
			if (tarjanNodes[j]->id == adgEdges[i]->from_node_name) {
				tEdge->from	= tarjanNodes[j];
			}
			if (tarjanNodes[j]->id == adgEdges[i]->to_node_name) {
				tEdge->to 		= tarjanNodes[j];
			}
		}
		if (tEdge->from == NULL || tEdge->to == NULL) {
			fprintf(stderr, "ERROR: Initializing Tarjan edge with adge edge: %s is not successful!!\n",
					isl_id_get_name(adgEdges[i]->name));
			return ret;
		}

		tarjanEdges.push_back(tEdge);
	}

	// 3. initialize graph
	Tarjan::Graph *tarjanGraph = new Tarjan::Graph;
	tarjanGraph->nodes = tarjanNodes;
	tarjanGraph->edges = tarjanEdges;

	//std::cout << "run tarjan" <<std::endl;
	Tarjan::SCCs_t foundSCCs;
	Tarjan::TarjanClass tc(tarjanGraph);
	tc.runTarjansAlgorithm(tarjanNodes[0], foundSCCs);
	//std::cout << "finish tarjan" <<std::endl;

	/* convert found SCCs in tarjan data structure back to ADG data structure */
	for (int i = 0; i < foundSCCs.size(); ++i) {
		ADGgraphSCC adgSCC;
		std::vector<Tarjan::Node*> tarjanSCC = foundSCCs[i];
		for (int j = 0; j < tarjanSCC.size(); ++j) {
			Node *adgNode = getNode(tarjanSCC[j]->id);
			adgSCC.push_back(adgNode);
		}
		ret.push_back(adgSCC);
	}

	// delete the allocated tarjan nodes and edges
	delete tarjanGraph;

	 return ret;
}


//// Phase/pattern computation

void ADG_helper::initPhases() {
  // Allocate phases for each port
  for (unsigned int i = 0; i < getChannels().size(); ++i) {
    Channel *ch = getChannels()[i];
    phases[(ch->from_port_name)] = new std::vector<short>;
    phases[(ch->to_port_name)] = new std::vector<short>;
  }
}

/* project out control variables in bounds of adg_domain to obtain
 * the representation with existential variables
 * */
__isl_give isl_set*
projectCtrlVars(const adg_domain *adg_dom){
  isl_set *set = isl_set_copy(adg_dom->bounds);

  if (adg_dom->controls.size() == 0) {
    return set;
  }

  unsigned nr_ctrl_vars = adg_dom->controls.size();
  unsigned nr_dim = isl_set_dim(set, isl_dim_set);

  // project out control variables from bounds
  set = isl_set_project_out(set, isl_dim_set, nr_dim- nr_ctrl_vars, nr_ctrl_vars);
  return set;
}

/* check all dependent dimensions
 * if the corresponding coefficients are non-zero in this constraint
 * */
int
check_ind_constratint(__isl_take isl_constraint *constrnt, void *user){
  struct ind_dims_t *ind_dims = (ind_dims_t *)user;
  //  std::cout << "nr. dim:" << nr_dims << std::endl;

  std::map<unsigned int, bool> dim_deps;

  isl_int iter_coffi, zero_int;
  isl_int_init(iter_coffi);
  isl_int_init(zero_int);
  isl_int_set_si(zero_int, 0);
  unsigned int nr_dep = 0;
  unsigned int nr_dims = ind_dims->dims;
  // 2 dimensions are dependent if the corresponding coefficient is non-zero
  for (int i = 0; i < nr_dims; ++i) {
    isl_constraint_get_coefficient(constrnt, isl_dim_set, i, &iter_coffi);

    /* if the coefficients of set and dimensions in port domain is not equal to those in process domain,
     * this means, port domain on the set domain is a subset of the set dimension in the process domain.
     * This dimension cannot be projected later on.
     */
    if (isl_int_eq(iter_coffi, zero_int)) {
      dim_deps[i] = false;
    }else{
      // the coefficient is non-zero
      dim_deps[i] = true;
      nr_dep ++;
    }
  }
  isl_int_clear(zero_int);
  isl_int_clear(iter_coffi);


  // at least two dimensions are dependent
  // store all dependent dimensions
  if (nr_dep > 1){
    for (int i = 0; i < nr_dims; ++i) {
      if (dim_deps[i] == false) continue;

      ind_dims->dim_dep_map[i] = true;
    }
  }


  isl_constraint_free(constrnt);
  return 0;
}

int
check_ind_dims(__isl_take isl_basic_set *domain_bset, void *user){

  ind_dims_t *ind_dims = (ind_dims_t *)user;

  /*std::cout << "nr. dim:" << nr_dims << std::endl;
  std::cout << "the domain: ";
  PRINTER = isl_printer_print_basic_set(PRINTER, domain_bset);
  PRINTER = isl_printer_end_line(PRINTER);*/

  int rt = isl_basic_set_foreach_constraint(domain_bset, &check_ind_constratint, ind_dims);
  assert(rt == 0);

  /* first n dimensions must be projected out.
   * If the domain is already dependent on the first dimension, no variant domain can be found.*/
  int first_dim = -1;
  int n_dim = 0;
  if (ind_dims->dim_dep_map[0] == false) {
    // The first dimension is independent.
    first_dim = 0;
//    std::cout << "first dim. to project out is:" << first_dim << std::endl;
  }

  // port domain shoud be live in the same space with node domain
  unsigned int nr_dims = ind_dims->dims;
  for (int i = 0; i < nr_dims; ++i) {
    if (ind_dims->dim_dep_map[i] == true) {
//      std::cout << i << " dimension is dependent" << std::endl;
    }else{
      n_dim++;
    }
  }

  // all dimensions are independent, we need to keep at one,
  // therefore, we only project out the outer-most "nr_dims-1" dimensions
  if (nr_dims == n_dim) {
    n_dim--;
  }
  //std::cout << "project out dims: " << n_dim << std::endl;


  // project outer-most "n_dim" independent dimensions
  if (first_dim != -1) {
    ind_dims->nr_ind_dim = n_dim;
  }

  isl_basic_set_free(domain_bset);

  return 0;
}

int
ADG_helper::getCommonPortsDims(__isl_keep isl_set *process_domain, Ports &ports, ind_dims_t &ind_dims){
  int nr_common_dims = INT_MAX;

  for (int i = 0; i < ports.size(); ++i) {
    // ignore self-edges
    if (isSelfEdge(getEdge(ports[i]->edge_name))) {
      continue;
    }

    isl_set *port_domain = getPDGDomain(ports[i]->domain);
//    std::cout << "in port domain: ";
//    PRINTER = isl_printer_print_set(PRINTER, port_domain);
//    PRINTER = isl_printer_end_line(PRINTER);

    ind_dims.nr_ind_dim = -1;
    // find the variant domain of this port domain and store in "ind_dims->var_domain"
    int rt = isl_set_foreach_basic_set(port_domain, check_ind_dims, &ind_dims);
    assert(rt == 0);

    /* find the common variant domain:
     * If the common variant domain is a superset of the currently found variant domain,
     * i.e. dimension of the common variant domain is larger than the currently found variant domain,
     * then assign it as the new common variant domain
     */
    if ( ind_dims.nr_ind_dim <= 0) {
      nr_common_dims = 0;
      isl_set_free(port_domain);

      return nr_common_dims;
    }
    if (ind_dims.nr_ind_dim < nr_common_dims) {
      nr_common_dims = ind_dims.nr_ind_dim;
    }

    /* step 2: iterate over all found common and outer-dimensions.
     * All port domain must be fully defined in the node domain at that dimension,
     * if a pattern can be possibly found.
     * */
    int nr_equal_dims = 0;
    for (int j = 0; j < nr_common_dims; ++j) {
      if (!isDimsEqual(process_domain, port_domain, 0, j+1)) {
        nr_equal_dims = j;
        break;
      } else {
        nr_equal_dims++;
      }
    }

    if (nr_equal_dims < nr_common_dims){
      nr_common_dims = nr_equal_dims;
    }
    assert(nr_common_dims != INT_MAX && nr_common_dims >=0);
    isl_set_free(port_domain);
  } // end ports

  return nr_common_dims;
}


var_domain_t *
ADG_helper::findVariantDomain2(const Process *process){
  isl_set *process_domain = getPDGDomain(process->domain);
//  std::cout << "process domain: ";
//  PRINTER = isl_printer_print_set(PRINTER, process_domain);
//  PRINTER = isl_printer_end_line(PRINTER);

  /* step 1: check the dependences between dimensions in each port domain.
   *  if a outer-dimension depends on a certain inner-dimension,
   *  no pattern can be found at the outer-dimension.
   *  NOTE: while(1) is naturally handled.
   *  */
  int nr_common_dims = INT_MAX;
  struct ind_dims_t ind_dims;
  unsigned int nr_dims = isl_set_dim(process_domain, isl_dim_set);
  ind_dims.dims = nr_dims;

  // iterate over all input port domains
  Ports input_ports = getInPorts(process);
  nr_common_dims = getCommonPortsDims(process_domain, input_ports, ind_dims);

//  // iterate over all output port domains
  Ports output_ports = getOutPorts(process);
  int nr_common_out_dims = getCommonPortsDims(process_domain, output_ports, ind_dims);
  if (nr_common_dims > nr_common_out_dims) {
    nr_common_dims = nr_common_out_dims;
  }
  // number should be between 0 and number of dimensions of the process domain
//  std::cout << "nr common dimension in variant domains: " << nr_common_dims << std::endl;


  // process the process domain and store its processed variant domain
//  std::cout << "process domain :";
//  PRINTER = isl_printer_print_set(PRINTER, process_domain);
//  PRINTER = isl_printer_end_line(PRINTER);
  if (nr_common_dims > 0) {
    // NOTE: currently the tuple is removed if certain dimensions of a set is projected out.
    // Therefore, we need id to reset it in the new set
    isl_id *process_name = isl_set_get_tuple_id(process_domain);
    isl_set *var_domain_process = isl_set_project_out((process_domain), isl_dim_set,
          0, nr_common_dims);
    var_domain_process = isl_set_set_tuple_id(var_domain_process, process_name);
    var_domains[process->name] = var_domain_process;
  } else {
    // no common variant domain is found
    var_domains[process->name] = process_domain;
  }
//  std::cout << "process variant domain :";
//  PRINTER = isl_printer_print_set(PRINTER, var_domains[process->name]);
//  PRINTER = isl_printer_end_line(PRINTER);


  // for each input port, project out independent dimensions from the port domain
  for (int i = 0; i < input_ports.size(); ++i) {
    // ignore self-edges
//    if (isSelfEdge(getEdge(input_ports[i]->edge_name))) {
//      continue;
//    }

    isl_set *port_domain = getPDGDomain(input_ports[i]->domain);

    // project out independent dimensions
    if (nr_common_dims > 0) {
      isl_id *port_name = isl_set_get_tuple_id(port_domain);
      isl_set *var_domain_port = isl_set_project_out((port_domain), isl_dim_out,
                0, nr_common_dims);
      var_domain_port = isl_set_set_tuple_id(var_domain_port, port_name);
      var_domains[input_ports[i]->name] = var_domain_port;
    } else {
      var_domains[input_ports[i]->name] = port_domain;
    }
//    std::cout << "in port variant domain :";
//    PRINTER = isl_printer_print_set(PRINTER, var_domains[input_ports[i]->name]);
//    PRINTER = isl_printer_end_line(PRINTER);
  }

  // for each output port, project out independent dimensions from the port domain
  for (int i = 0; i < output_ports.size(); ++i) {
    // ignore self-edges
    if (isSelfEdge(getEdge(output_ports[i]->edge_name))) {
      continue;
    }

    isl_set *port_domain = getPDGDomain(output_ports[i]->domain);

    // first project out independent dimensions
    if (nr_common_dims > 0) {
      isl_id *port_name = isl_set_get_tuple_id(port_domain);
      isl_set *var_domain_port = isl_set_project_out(port_domain, isl_dim_out,
                0, nr_common_dims);
      var_domain_port = isl_set_set_tuple_id(var_domain_port, port_name);
      var_domains[output_ports[i]->name] = var_domain_port;
    } else {
      var_domains[output_ports[i]->name] = port_domain;
    }
//    std::cout << "-->port variant domain :";
//    PRINTER = isl_printer_print_set(PRINTER, var_domains[output_ports[i]->name]);
//    PRINTER = isl_printer_end_line(PRINTER);
  }

  assert(var_domains.empty() == false);
  return &var_domains;
}


/* the length of the phases is equal to the number of integer points in the variant domain
 * Note that the variant domain is non-parameterized, therefore, this function also return a constant
 */
unsigned int
ADG_helper::getPhaseLength(__isl_keep isl_set *var_domain){
  return getCardinality(var_domain);
}


/* compute phases for each port of the process according to the max variant domain
 * */
phases_t *
ADG_helper::computePhases(const Process *process){
  // variant domains should be generated before hand
  assert(var_domains.size() > 0);
  assert(var_domains.count(process->name) > 0);
  isl_set *process_var_domain = var_domains[process->name];
  /*std::cout << "process variant domain: ";
  PRINTER = isl_printer_print_set(PRINTER, process_var_domain);
  PRINTER = isl_printer_end_line(PRINTER);*/

  // scan the process domain according to lexicographical order
  isl_set *process_dom_new = isl_set_copy(process_var_domain);
  while(isl_set_is_empty(process_dom_new) != 1){
    // get the lexicographical point
    isl_set *lexmin_process_dom = isl_set_lexmin(isl_set_copy(process_dom_new));

    // iterate over all input port domains
    Ports input_ports = getInPorts(process);
    for (int i = 0; i < input_ports.size(); ++i) {
      isl_id *port_id = input_ports[i]->name;
      assert(var_domains.count(port_id) > 0);
      isl_set *port_var_domain = var_domains[port_id];

      if (isl_set_is_subset(lexmin_process_dom, port_var_domain)) {
        //std::cout << "1" << std::endl;
        phases[port_id]->push_back(1);
      } else {
        //std::cout << "0" << std::endl;
        phases[port_id]->push_back(0);
      }
    } // end input ports

    // iterate over all output port domains
    Ports output_ports = getOutPorts(process);
    for (int i = 0; i < output_ports.size(); ++i) {
      isl_id *port_id = output_ports[i]->name;
      assert(var_domains.count(port_id) > 0);
      isl_set *port_var_domain = var_domains[port_id];

      if (isl_set_is_subset(lexmin_process_dom, port_var_domain)) {
        //std::cout << "1" << std::endl;
        phases[port_id]->push_back(1);
      } else {
        //std::cout << "0" << std::endl;
        phases[port_id]->push_back(0);
      }
    } // end output ports


    // remove lexmin point from the set
    process_dom_new = isl_set_subtract(process_dom_new, lexmin_process_dom);
  }
  isl_set_free(process_dom_new);

  return &phases;
}


/* check if the process has a simple consumption/production pattern, such as [1].
 * Basic idea is that, if all input/output port domains are equal to process domain,
 * all consumption/production patterns have only one phase with rate 1.
 *
 * If the process has a simple consumption/production pattern, store the pattern [1] for all input/output ports */
bool
ADG_helper::checkSimplePattern(const Process *process){
  bool isSimplePattern = true;
//  std::cout << "process domain: ";
//  PRINTER = isl_printer_print_set(PRINTER, process->domain->bounds);
//  PRINTER = isl_printer_end_line(PRINTER);

  isl_set *unwrapped_procs_domain = getPDGDomain(process->domain);
//  std::cout << "projected process domain: ";
//  PRINTER = isl_printer_print_set(PRINTER, unwrapped_procs_domain);
//  PRINTER = isl_printer_end_line(PRINTER);

  // check all input ports
  Ports input_ports = process->input_ports;
  for (int i = 0; i < input_ports.size(); ++i) {
    // ignore self-edges first
    if (isSelfEdge(getEdge(input_ports[i]->edge_name))) {
      continue;
    }

    isl_set *unwrapped_port_domain = getPDGDomain(input_ports[i]->domain);
//    std::cout << "in port domain: ";
//    PRINTER = isl_printer_print_set(PRINTER, input_ports[i]->domain->bounds);
//    PRINTER = isl_printer_end_line(PRINTER);

    if (isl_set_is_equal(unwrapped_port_domain, unwrapped_procs_domain) != 1) {
      isSimplePattern = false;
    }

    isl_set_free(unwrapped_port_domain);
  } // end input ports

  if (!isSimplePattern) {
    isl_set_free(unwrapped_procs_domain);
    return isSimplePattern;
  }


  // check all output ports
  Ports output_ports = process->output_ports;
  for (int i = 0; i < output_ports.size(); ++i) {
    // ignore self-edges first
    if (isSelfEdge(getEdge(output_ports[i]->edge_name))) {
      continue;
    }

    isl_set *unwrapped_port_domain = getPDGDomain(output_ports[i]->domain);
//    std::cout << "out port domain: ";
//    PRINTER = isl_printer_print_set(PRINTER, output_ports[i]->domain->bounds);
//    PRINTER = isl_printer_end_line(PRINTER);

    if (isl_set_is_equal(unwrapped_port_domain, unwrapped_procs_domain) != 1){
      isSimplePattern = false;
    }

    isl_set_free(unwrapped_port_domain);
  } // end output ports

//  std::cout<< "it is a simple pattern." << std::endl;
  isl_set_free(unwrapped_procs_domain);
  if (!isSimplePattern) {
      return isSimplePattern;
  }

  // simple pattern, store the pattern [1]
  for (int i = 0; i < input_ports.size(); ++i) {
    // ignore self-edges first
    if (isSelfEdge(getEdge(input_ports[i]->edge_name))) {
      continue;
    }

    isl_id *port_id = input_ports[i]->name;
    phases[port_id]->push_back(1);
  }
  for (int i = 0; i < output_ports.size(); ++i) {
    // ignore self-edges first
    if (isSelfEdge(getEdge(output_ports[i]->edge_name))) {
      continue;
    }

    isl_id *port_id = output_ports[i]->name;
    phases[port_id]->push_back(1);
  }

  return isSimplePattern;
}

// Write phases belonging to given port to ostream.
// Requires that computePhases has been called before.
void
ADG_helper::writePhase(isl_id *portName, std::ostream &strm, char sep){
	std::vector<short> *phases_port = phases[portName];
	int len_phases = phases_port->size();
	assert(len_phases > 0);

	strm << (*phases_port)[0];
	for (int i = 1; i < len_phases; i++) {
		strm << sep << (*phases_port)[i];
	}
}

} // end namespace adg_helper
